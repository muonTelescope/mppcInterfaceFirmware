#include "nau7802/NAU7802.h"
#include "nau7802/NAU7802.cpp"
#include <Wire.h>
#include <SPI.h>
#include <SoftwareWire.h>
#include <math.h>

#define i2cSlaveAdress 0x08

#define TCAADDR 0x70

#define ENUMERATE
// #define REGMAP
#define SERIALINTERFACE
// #define DEBUG

// Production A config
#define led 9
#define SCL A0
#define SDA A1

// Thermistor
#define BCOEFFICIENT 4250
#define THERMISTORNOMINAL 100000
#define TEMPERATURENOMINAL 25
#define SERIESRESISTOR 200000.0

// Temprature coefficent or Vop (mV/C)
#define dT_Vop 54

static uint8_t csPins[] = {8, 7, 6, 5};
// Calibrate voltage offsett out of  
static float calibrationFactor[] = {1.0, 1.0, 1.0, 1.0};
// Voltage range , {{min0,max0},{min1,max1}...}
static float voltageRange[][2] = {{52.5, 57.5},
                                  {52.5, 57.5},
                                  {52.5, 57.5},
                                  {52.5, 57.5}};

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
long readMicroVolts(uint8_t channel, uint8_t numSamples);
long readMicroCentigrade(uint8_t channel, uint8_t numSamples);
void setByte(uint8_t byte, uint8_t channel);
int setVoltage(float voltage, uint8_t channel);

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
  // Initilaize SPI HV pins, and ADCs assume each HV has ADC
  for (int i = 0; i < sizeof(csPins) / sizeof(csPins[0]) ; i++) {
    
    #ifdef ENUMERATE
    Serial.print("Initalize Channel ");
    Serial.println(i);
    #endif //ENUMERATE

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

  // Trun all channels off
  for(int i = 0 ; i < 4 ; i++ ){
      setByte(i, 0);
  }
}

void loop(void) {

  #ifdef REGMAP
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
  #endif //REGMAP

  #ifdef SERIALINTERFACE
  // Set the voltage when our recive a float
  if (Serial.available() > 0) {
    // Read the channel and voltage requested from serial line
    long channel = Serial.parseInt();
    float voltage = Serial.parseFloat();
    // Remove remaining charachets from serial pool.
    while (Serial.available() > 0) {
      Serial.read();
    }
    // Print out request
    Serial.print("Requested Channel ");
    Serial.print(channel);
    Serial.print(" Voltage: ");
    Serial.println(voltage, 6);

    // If channel number is not availible, let the user know
    if((channel > sizeof(csPins)/sizeof(csPins[0])) | channel < 0){
      Serial.print("Channel out of range, this device only has ");
      Serial.print((sizeof(csPins)/sizeof(csPins[0]))+1);
      Serial.println(" channels.");
    } else if (setVoltage(voltage, channel) == 0){
      delay(500);
      float measuredVoltage = readMicroVolts(channel,10) / 1000000.0;
      float error = ((abs(voltage - measuredVoltage)) / (voltage)) * 100;
      Serial.print("Voltage: ");
      Serial.print(measuredVoltage,4);
      Serial.print(" Yields error of: ");
      Serial.print(error, 4);
      Serial.println("%.");
    } else {
      Serial.print("Error, out of range: ");
      Serial.print(voltageRange[0][0], 3);
      Serial.print(" V - ");
      Serial.print(voltageRange[0][1], 3);
      Serial.println(" V");
    }
  }
  #endif //SERIALINTERFACE

  for (int i = 0; i < sizeof(csPins) / sizeof(csPins[0]); i++) {
    float target = writeV[i]/1000000.0;
    // Try and set the voltage
    if(setVoltage(target, i) != 0){
      // Set the voltage to the max if request exceeds max by more than 1V
      // Set the voltage to the min if request below min by less than 1V
      // Otherwise turn off
      if((target > (voltageRange[i][1] + 1))|(target < (voltageRange[i][0] - 1))){
        setByte(0,i);
      } else if(target > voltageRange[i][1]){
        setVoltage(voltageRange[i][1],i);
      } else if (target < voltageRange[i][0]) {
        setVoltage(voltageRange[i][0],i);
      }
    }
  }

  // Update voltage and temprature registers
  for (int i = 0; i < sizeof(csPins) / sizeof(csPins[0]); i++) {
    readV[i] = readMicroVolts(i,10);
    readT[i] = readMicroCentigrade(i,10);
  }

  for (int i = 0; i < sizeof(csPins) / sizeof(csPins[0]) ; i++) {
    Serial.print(readT[i]/1000000.0,6);
    Serial.print(",");
  }
    Serial.println("");  

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

void tcaselect(uint8_t i) {
  if (i > 7) {return;}
  wire.beginTransmission(TCAADDR);
  wire.write(1 << i);
  wire.endTransmission();
}

// Compensates voltage for temprature variaiton
float compensateVoltage(float voltageAt25C, float temp){
  float tempDiff = temp-25.0;
  return (voltageAt25C + (tempDiff * dT_Vop * 0.001)); // Convert to mV, multiply by diffrence in C
}

// Given a voltage, set it on the specifc channel
int setVoltage(float voltage, uint8_t channel) {
  float min = voltageRange[channel][0];
  float max = voltageRange[channel][1];
  float byte = 0;
  byte = max - voltage;
  byte *= (254 / (max - min));
  byte++;
  byte = round(byte);
  if (byte < 1 || byte > 255) {
    return 1;
  } else {
    setByte((uint8_t)byte, channel);
    return 0;
  }
}

// Set the byte for setting the voltage
void setByte(uint8_t byte, uint8_t channel) {
  digitalWrite(csPins[channel], LOW);
  SPI.transfer(byte);
  digitalWrite(csPins[channel], HIGH);
}

// Read the voltage from a specific channel, averagd over numSamples
long readMicroVolts(uint8_t channel, uint8_t numSamples) {
  tcaselect(channel);  // Change I2C Switch to correct channel  
  adc.selectCh1();     // Channel 1 has 50x Divided High voltage
  adc.readmV();
  adc.readmV();
  adc.readmV();
  adc.readmV();
  float average = 0;
  for (int i = 0; i < numSamples; i++) {
    average += adc.readmV();
  }
  average /= numSamples;
  return (long)(average * 50.0 * calibrationFactor[channel] * 1000000.0);  // 1000000x Convert to microvolts
}

// Read temprature from thermistor attached, averaged over numSamples
long readMicroCentigrade(uint8_t channel, uint8_t numSamples) {
  tcaselect(channel);  // Change I2C Switch to correct channel  
  adc.selectCh2();     // Channel 2 has thermistor
  adc.readmV();
  adc.readmV();
  adc.readmV();
  adc.readmV();
  float average = 0;
  for (int i = 0; i < numSamples; i++) {
    average += adc.readADC();
  }
  average /= numSamples;
  float steinhart;
  steinhart = (average * SERIESRESISTOR) /
              (16777216 - average);                  // Resistance of the thermistor
  steinhart = steinhart / THERMISTORNOMINAL;         // (R/Ro)
  steinhart = log(steinhart);                        // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                         // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);  // + (1/To)
  steinhart = 1.0 / steinhart;                       // Invert
  steinhart -= 273.15;                               // Convert to C
  return (steinhart * 1000000);                      // Return microCentigrade
}