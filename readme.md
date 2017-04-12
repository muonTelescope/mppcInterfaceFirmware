# MPPC Interface Firmware
Communication through I2C the following shows the register map

## Register Map
<table>
  <tr>
    <td>Address</td>
    <td>Byte 7</td>
    <td>Byte 6</td>
    <td>Byte 5</td>
    <td>Byte 4</td>
    <td>Byte 3</td>
    <td>Byte 2</td>
    <td>Byte 1</td>
    <td>Byte 0</td>
  </tr>
  <!--Read Voltages-->
  <tr>
    <td>0x00</td>
    <td colspan="8">Channel 0 Voltage (in uV) 31:24</td>
  </tr>

  <tr>
    <td>0x01</td>
    <td colspan="8">Channel 0 Voltage (in uV) 23:16</td>
  </tr>

  <tr>
    <td>0x02</td>
    <td colspan="8">Channel 0 Voltage (in uV) 15:8</td>
  </tr>

  <tr>
    <td>0x03</td>
    <td colspan="8">Channel 0 Voltage (in uV) 7:0</td>
  </tr>

  <tr>
    <td>0x04</td>
    <td colspan="8">Channel 1 Voltage (in uV) 31:24</td>
  </tr>

  <tr>
    <td>0x05</td>
    <td colspan="8">Channel 1 Voltage (in uV) 23:16</td>
  </tr>

  <tr>
    <td>0x06</td>
    <td colspan="8">Channel 1 Voltage (in uV) 15:8</td>
  </tr>

  <tr>
    <td>0x07</td>
    <td colspan="8">Channel 1 Voltage (in uV) 7:0</td>
  </tr>

  <tr>
    <td>0x08-0x1F</td>
    <td colspan="8">...</td>
  </tr>

  <!--Read Tempratures-->
  <tr>
    <td>0x20</td>
    <td colspan="8">Channel 0 Temprature (in uC) 31:24</td>
  </tr>

  <tr>
    <td>0x21</td>
    <td colspan="8">Channel 0 Temprature (in uC) 23:16</td>
  </tr>

  <tr>
    <td>0x22</td>
    <td colspan="8">Channel 0 Temprature (in uC) 15:8</td>
  </tr>

  <tr>
    <td>0x23</td>
    <td colspan="8">Channel 0 Temprature (in uC) 7:0</td>
  </tr>

  <tr>
    <td>0x24</td>
    <td colspan="8">Channel 1 Temprature (in uC) 31:24</td>
  </tr>

  <tr>
    <td>0x25</td>
    <td colspan="8">Channel 1 Temprature (in uC) 23:16</td>
  </tr>

  <tr>
    <td>0x26</td>
    <td colspan="8">Channel 1 Temprature (in uC) 15:8</td>
  </tr>

  <tr>
    <td>0x27</td>
    <td colspan="8">Channel 1 Temprature (in uC) 7:0</td>
  </tr>

  <tr>
    <td>0x28-0x3F</td>
    <td colspan="8">...</td>
  </tr>
  
  <!--Write Registers-->
  <tr>
    <td>0x40</td>
    <td colspan="8">Write Channel 0 Voltage (in uV) 31:24</td>
  </tr>

  <tr>
    <td>0x41</td>
    <td colspan="8">Write Channel 0 Voltage (in uV) 23:16</td>
  </tr>

  <tr>
    <td>0x42</td>
    <td colspan="8">Write Channel 0 Voltage (in uV) 15:8</td>
  </tr>

  <tr>
    <td>0x43</td>
    <td colspan="8">Write Channel 0 Voltage (in uV) 7:0</td>
  </tr>

  <tr>
    <td>0x44</td>
    <td colspan="8">Write Channel 1 Voltage (in uV) 31:24</td>
  </tr>

  <tr>
    <td>0x45</td>
    <td colspan="8">Write Channel 1 Voltage (in uV) 23:16</td>
  </tr>

  <tr>
    <td>0x46</td>
    <td colspan="8">Write Channel 1 Voltage (in uV) 15:8</td>
  </tr>

  <tr>
    <td>0x47</td>
    <td colspan="8">Write Channel 1 Voltage (in uV) 7:0</td>
  </tr>

  <tr>
    <td>0x48-0x5F</td>
    <td colspan="8">...</td>
  </tr>
  <tr>
    <td>0x60</td>
    <td>LED Flag</td>
    <td>NULL</td>
    <td>NULL</td>
    <td>NULL</td>
    <td>NULL</td>
    <td>NULL</td>
    <td>NULL</td>
    <td>NULL</td>
  </tr>

</table>