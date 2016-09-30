#include "nau7802/NAU7802.h"
#include "nau7802/NAU7802.cpp"
#include <Wire.h>
#include <SPI.h>
#include <SoftwareWire.h>
#include <math.h>

#define i2cSlaveAdress 8

#define TCAADDR 0x70

// // Breadboard Config
// #define led 8
// #define SCL 2
// #define SDA 3

// int csPins[] = {6, 5, 4, 7};

// Production A config
#define led 8
#define SCL A0
#define SDA A1

int csPins[] = {2, 3, 4, 5};

long readV[8];
long readT[8];
long writeV[8];
uint8_t currentRegister = 0x00;
uint8_t voltageByte[] = {0, 0, 0, 0, 0, 0, 0, 0};

typedef struct registerMap {
  // Read voltage
  long readV0;
  long readV1;
  long readV2;
  long readV3;
  long readV4;
  long readV5;
  long readV6;
  long readV7;
  // Read Temprature
  long readT0;
  long readT1;
  long readT2;
  long readT3;
  long readT4;
  long readT5;
  long readT6;
  long readT7;
  // Write voltage
  long writeV0;
  long writeV1;
  long writeV2;
  long writeV3;
  long writeV4;
  long writeV5;
  long writeV6;
  long writeV7;
  // Led flag
  bool ledFlag;
  // NULL Saved for future
  char null[159];
};

void tcaselect(uint8_t i);
long readMicroVolts(uint8_t i);
long readMicroCentigrade(uint8_t i);
void setVoltage(byte volts, int csPin);

// Create the NAU7802 ADC object
NAU7802 adc = NAU7802();
SoftwareWire wire = SoftwareWire(SDA, SCL);
// Create the register object
registerMap registers;

void setup(void) {
  Serial.begin(9600);
  Serial.println("Begin");
  // Begin acting as I2C slave
  Wire.begin(i2cSlaveAdress);    // join i2c bus with address #8
  Wire.onReceive(receiveEvent);  // Getting Data
  Wire.onRequest(requestEvent);  // Being asked for data  
  // Also Begin acting as I2C master
  wire.begin();

  // Initalize all ADCs assume each HV has ADC
  // Initilaize SPI HV pins
  for (int i = 0; i < sizeof(csPins) / sizeof(int); i++) {
    // I2C ADC
    tcaselect(i);
    
    adc.begin(SDA, SCL);
    adc.avcc2V4();
    // SPI HV
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
}

void loop(void) {
  // Debugging Print Register map
  struct registerMap* strucPtr = &registers;
  unsigned char* charPtr = (unsigned char*)strucPtr;
  int i;
  // Serial.print("structure size : ");
  // Serial.print(sizeof(struct registerMap));
  // Serial.print(" bytes\n");
  for (i = 0; i < sizeof(struct registerMap); i++) {
    if (charPtr[i] < 16) {
      Serial.print("0");
    }
    Serial.print(charPtr[i], HEX);
  }
  Serial.println();
  // End Debugging

  // Check if voltage needs to be changed
  // Slow and pretty terrible closed loop control
  //  Oscilates arround correct value
  //  Changes once per cycle
  for (int i = 0; i < sizeof(csPins) / sizeof(int); i++) {
    if (readV[i] < 50000000 &&
        writeV[i] > 50000000) {  // If the HV module was off this will turn it
                                 // back on at it lowest value
      voltageByte[i] = 255;
    }
    if (writeV[i] > readV[i] && writeV[i] > 50000000) {
      // Current Voltage too Low, Decrese Byte
      if (voltageByte[i] != 0 && voltageByte[i] != 1) {
        voltageByte[i]--;
      }
      setVoltage(voltageByte[i], csPins[i]);

    } else if (writeV[i] < readV[i] && writeV[i] > 50000000) {
      // Current Voltage too High, Increse Byte
      if (voltageByte[i] != 255) {
        voltageByte[i]++;
      }
      setVoltage(voltageByte[i], csPins[i]);
    } else if (writeV[i] < 50000000) {  // Turn off HV Module if voltage
                                        // requested is less than 50V
      setVoltage(0, csPins[i]);
    }
  }

  // Update voltage and temprature registers
  for (int i = 0; i < sizeof(csPins) / sizeof(int); i++) {
    readV[i] = readMicroVolts(i);
    readT[i] = readMicroCentigrade(i);
  }

  // Dump readV and readT to registerStruct
  memcpy(&registers.readV0, &readV, sizeof(readV));
  memcpy(&registers.readT0, &readT, sizeof(readT));
  // Dump Requested voltages to writeV
  memcpy(&writeV, &registers.writeV0, sizeof(writeV));

  // If the blink led flag is on, blink
  if (registers.ledFlag) {
    digitalWrite(led, HIGH);
    digitalWrite(led, LOW);
    // Turn flag off
    registers.ledFlag = 0;
  }
}

void receiveEvent(int numBytes) {
  // change register pointer
  currentRegister = Wire.read();
  // Then write all subsequent bytes into current register
  while (Wire.available()) {
    if (currentRegister > 63) {  // Do not write in the read sections
      *((unsigned char*)&registers + currentRegister) = Wire.read();
    } else {  // If someone tries, discard that data
      Wire.read();
    }
    currentRegister++;
  }
}

void requestEvent() {
  // Respond with the byte in that register
  Wire.write(*((unsigned char*)&registers + currentRegister));
  currentRegister++;
}

void setVoltage(byte volts, int csPin) {
  digitalWrite(csPin, LOW);
  SPI.transfer(volts);
  digitalWrite(csPin, HIGH);
}

void tcaselect(uint8_t i) {
  if (i > 7) return;

  wire.beginTransmission(TCAADDR);
  wire.write(1 << i);
  wire.endTransmission();
}

long readMicroVolts(uint8_t i) {
  tcaselect(i);     // Change I2C Switch to correct channel
  adc.selectCh1();  // Channel 1 has 50x Divided High voltage
  return adc.readmV() * 50 * 1000000;  // 1000000x Convert to microvolts
}

long readMicroCentigrade(uint8_t i) {
  tcaselect(i);     // Change I2C Switch to correct channel
  // adc.selectCh2();  // Channel 2 has thermistor
  // Need to implement conversion to temprature
  float temp = 100000.0 / ((8388608.0 / (adc.readmV()) - 1));
  temp = -log(temp);
  temp = 1 / (0.001129148 + (0.000234125 * temp) +
              (0.0000000876741 * temp * temp * temp));
  return (temp - 273.15) * 1000000;  // convert to microCentigrade
}