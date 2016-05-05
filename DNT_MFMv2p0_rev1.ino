/**************************************************************************/
/*!
    @file     MFM_v2.0
    Based on  Adafruit MCP4725 DAC demo code      sinewave.pde
              Arduino blink                       blink.ino
              mcp4728 library                     BasicUse.ino
              
    @author   Eyal Aklimi, Diamond Nanotechnologies 2016
    @license (original code)  BSD (see license.txt)

    This code will operate DNT_MFM v2.0

    Serial communication reference (send newline \n at the end of each command):
    D: Enter DebugMode. Default is debugmode=off. Also print status.
    X: Exit DebugMode.
    SX####: set X channel current to #### in mA, range is 0~2100mA
    SY####: same for Y
    SZ####: same for Z
    R: reset / check LED
    C: Print DAC status
    ?: Identify module
    MX: measure current in X. return format #### in mA
    MY: same for Y
    MZ: same for Z
    
*/
/**************************************************************************/
#include <Wire.h>
#include "mcp4728.h"

mcp4728 dac = mcp4728(0); // instantiate mcp4728 object, Device ID = 0

String strInCommand;

int DebugMode = 0;

int LEDG_X = 3;
int LEDG_Y = 4;
int LEDG_Z = 5;

int SET_X = 3; // temporary as X channel on populated v2.0 board is damaged, DAC3 is wired to SETX instead of DAC0
int SET_Y = 1;
int SET_Z = 2; 

int SEN_X = 0; // as wired to arduino nano A0
int SEN_Y = 1; // as wired to arduino nano A1
int SEN_Z = 2; // as wired to arduino nano A2

void setup(void)
{
  Serial.begin(9600);
  Serial.println("DNT_MFM_v2.0 says HELLO. Send D for DebugMode.");

  pinMode(13, OUTPUT); // LED8 Yellow
  pinMode(8, OUTPUT); // LED9_GP Blue
  pinMode(7, OUTPUT); // LED10_GP Blue 
  pinMode(6, OUTPUT); // LED11_GP Blue
  pinMode(LEDG_X, OUTPUT); // LED_X Green
  pinMode(LEDG_Y, OUTPUT); // LED_Y Green
  pinMode(LEDG_Z, OUTPUT); // LED_Z Green
  
  dac.begin();  // initialize i2c interface
  dac.vdd(5000); // set VDD(mV) of MCP4728 for correct conversion between LSB and Vout

  analogReference(INTERNAL); // INTERNAL is 1.1v reference. DEFAULT (=EXTERNAL) is Vdd = 5V here.

  digitalWrite(LEDG_X, LOW);
  digitalWrite(LEDG_Y, LOW);
  digitalWrite(LEDG_Z, LOW);

  dac.voutWrite(0,0,0,0);
  dac.setPowerDown(0, 0, 0, 0);
  dac.setVref(1,1,1,1); // internal vref = 2.048v with gain=1

  fp_leds_init();
  
  // printStatus();
  
}

void loop(void)
{
  // "Blink"
  digitalWrite(8, HIGH);
  delay(100);
  digitalWrite(8, LOW);
  digitalWrite(7, HIGH);
  delay(100);  
  digitalWrite(7, LOW);
  digitalWrite(6, HIGH);
  delay(100);
  digitalWrite(6, LOW);
  digitalWrite(7, HIGH);
  delay(100);
  digitalWrite(7, LOW);

  if(Serial.available())   // something came across serial
  {
    char c = Serial.read();  
    
    if(c == '\n') // Received newline. parse command.
    {
      //Serial.write("nl");
      parseCommand(strInCommand);
      strInCommand = "";
    }
    else // Receive data, add char to string.
    {
      strInCommand += c;
      //Serial.write("c");
    }
  }
}

void parseCommand(String str)
{
  if(DebugMode) Serial.write("DNT_MFM_v2.0: ");
  String strCmd;
  String strVal;
  String strCha;
  
  int val;
  int tmpCurVal;
  strCmd = str.substring(0,1); // First letter 
  strCha = str.substring(1,2); // Second letter 
  strVal = str.substring(2); // Value 
  // note: not all values are mandatory, code won't fail.
  
  strCmd.toUpperCase(); // both non-case sensitive
  strCha.toUpperCase(); // both non-case sensitive
  
  val=constrain(strVal.toInt(),0,2100); // val in mA
  // Convert 0~2100mA to 0~750mV: simply divide by 2.8v/v
  val=val/2.8; // val is guaranteed to be 0~750mV

  if(strCmd.equals("S")) // ********************* SET CURRENT
  {
    if(DebugMode){
      Serial.print("Changing DAC to ");
      Serial.print(val);
      Serial.print("mV, which represents ");
      Serial.print(val*2.8);
      Serial.println("mA of current.");
    }
    if(strCha.equals("X"))
    {
      //change X value
      dac.voutWrite(SET_X, val);
      if(val==0)
        digitalWrite(LEDG_X, LOW);
      else
        digitalWrite(LEDG_X, HIGH);
    }
    else if(strCha.equals("Y"))
    {
      //change Y value
      dac.voutWrite(SET_Y, val);
      if(val==0)
        digitalWrite(LEDG_Y, LOW);
      else
        digitalWrite(LEDG_Y, HIGH);
    }
    else if(strCha.equals("Z"))
    {
      //change Z value
      dac.voutWrite(SET_Z, val);
      if(val==0)
        digitalWrite(LEDG_Z, LOW);
      else
        digitalWrite(LEDG_Z, HIGH);
    }
  Serial.println("1");
  }
  else if(strCmd.equals("M")) // ********************* MEASURE (SENSE) CURRENT
  {
    if(DebugMode) Serial.print("Measure current value (in mA) = ");
    if(strCha.equals("X"))
    {
      //sense X value
      tmpCurVal = analogRead(SEN_X);
      // value 0~1023 represents 0~1.1V. here 750mV represents 2100mA.
      // So 698 represents 750mV, and each 1 represents 1.07mV which is 3mA
      Serial.println(int(tmpCurVal*3),DEC);
    }
    else if(strCha.equals("Y"))
    {
      //sense Y value
      tmpCurVal = analogRead(SEN_Y);
      Serial.println(int(tmpCurVal*3),DEC);
    }
    else if(strCha.equals("Z"))
    {
      //sense Z value
      tmpCurVal = analogRead(SEN_Z);
      Serial.println(int(tmpCurVal*3),DEC);
    }
  }
  else if(strCmd.equals("?"))
  {
    // Identify
    Serial.println("DNT_MFM_v2.0");
  }
  else if(strCmd.equals("D"))
  {
    // Into Debug Mode
    Serial.println("Going into debug mode. Commands menu:");
    Serial.println("  X: exit debug mode");
    Serial.println("  R: Reset, and frontpanel LEDs check");
    Serial.println("  C: Print DAC registers status");
    Serial.println("  S*####: Set current in channel *=X, Y, or Z, to #### in mA");
    Serial.println("  M*: Measure current in channel *=X, Y, or Z. Response is in mA");
    Serial.println("  ?: Identify module");
    DebugMode=1;
    printStatus();
  }
  else if(strCmd.equals("X"))
  {
    // Out of Debug Mode
    Serial.println("Going out of debug mode.");
    DebugMode=0;
  }
  else if(strCmd.equals("R"))
  {
    // Reset
    if(DebugMode){
      Serial.println("Reset");
    }
    else {
      Serial.println("1");
    }
    fp_leds_init();
    dac.voutWrite(0,0,0,0);
    dac.setPowerDown(0, 0, 0, 0);
    dac.setVref(1,1,1,1);
  }
  else if(strCmd.equals("C"))
  {
    // Print DAC registers status
    printStatus();
  }
  else
  {
    if(DebugMode){
      Serial.println("COMMAND UNKNOWN");
    }
    else {
      Serial.println("0");
    }
  }
}


void printStatus()
{
  int id = dac.getId();
  Serial.print("Device ID = ");
  Serial.print(id,DEC);
  Serial.println(". DAC0=X, DAC1=Y, DAC2=Z");
  
  Serial.println("NAME     Vref  Gain  PowerDown  Value");
  for (int channel=0; channel <= 2; channel++)
  { 
    Serial.print("DAC");
    Serial.print(channel,DEC);
    Serial.print("   ");
    Serial.print("    "); 
    Serial.print(dac.getVref(channel),BIN);
    Serial.print("     ");
    Serial.print(dac.getGain(channel),BIN);
    Serial.print("       ");
    Serial.print(dac.getPowerDown(channel),BIN);
    Serial.print("       ");
    Serial.println(dac.getValue(channel),DEC);
  }
  Serial.println(" ");
}

int i;
void fp_leds_init()
{
  for(i=0; i<2; i++)
  {
    digitalWrite(LEDG_X, HIGH);
    delay(100);
    digitalWrite(LEDG_X, LOW);
    digitalWrite(LEDG_Y, HIGH);
    delay(100);  
    digitalWrite(LEDG_Y, LOW);
    digitalWrite(LEDG_Z, HIGH);
    delay(100);
    digitalWrite(LEDG_Z, LOW);
    digitalWrite(LEDG_Y, HIGH);
    delay(100);
    digitalWrite(LEDG_Y, LOW);
  }
   for(i=0; i<3; i++)
  {
    digitalWrite(LEDG_X, HIGH);
    digitalWrite(LEDG_Y, HIGH);
    digitalWrite(LEDG_Z, HIGH);
    delay(200);
    digitalWrite(LEDG_X, LOW);
    digitalWrite(LEDG_Y, LOW);
    digitalWrite(LEDG_Z, LOW);
    delay(200);
  }
}

  /* SAMPLE CODE FROM MCP4728 LIBRARY
   *  
  dac.analogWrite(500,500,500,500); // write to input register of DAC four channel (channel 0-3) together. Value 0-4095
  dac.analogWrite(0,700); // write to input register of a DAC. Channel 0-3, Value 0-4095
  int value = dac.getValue(0); // get current input register value of channel 0
  Serial.print("input register value of channel 0 = ");
  Serial.println(value, DEC); // serial print of value

  dac.voutWrite(1800, 1800, 1800, 1800); // write to input register of DAC. Value(mV) (V < VDD)
  dac.voutWrite(2, 1600); // write to input register of DAC. Channel 0 - 3, Value(mV) (V < VDD)
  int vout = dac.getVout(1); // get current voltage out of channel 1
  Serial.print("Voltage out of channel 1 = "); 
  Serial.println(vout, DEC); // serial print of value

            Voltage reference settings
                Vref setting = 1 (internal), Gain = 0 (x1)   ==> Vref = 2.048V
                Vref setting = 1 (internal), Gain = 1 (x2)   ==> Vref = 4.096V
                Vref setting = 0 (external), Gain = ignored  ==> Vref = VDD
  
  dac.setVref(1,1,1,1); // set to use internal voltage reference (2.048V)
  dac.setVref(0, 0); // set to use external voltage reference (=VDD, 2.7 - 5.5V)
  int vref = dac.getVref(1); // get current voltage reference setting of channel 1
  Serial.print("Voltage reference setting of channel 1 = "); // serial print of value
  Serial.println(vref, DEC); // serial print of value

  dac.setGain(0, 0, 0, 0); // set the gain of internal voltage reference ( 0 = gain x1, 1 = gain x2 )
  dac.setGain(1, 1); // set the gain of internal voltage reference ( 2.048V x 2 = 4.096V )
  int gain = dac.getGain(2); // get current gain setting of channel 2
  Serial.print("Gain setting of channel 2 = "); // serial print of value
  Serial.println(gain, DEC); // serial print of value

              Power-Down settings
                0 = Normal , 1-3 = shut down most channel circuit, no voltage out and saving some power.
                1 = 1K ohms to GND, 2 = 100K ohms to GND, 3 = 500K ohms to GND

  dac.setPowerDown(0, 0, 0, 0); // set Power-Down ( 0 = Normal , 1-3 = shut down most channel circuit, no voltage out) (1 = 1K ohms to GND, 2 = 100K ohms to GND, 3 = 500K ohms to GND)
  dac.setPowerDown(3, 1); // Power down channel 3 ( no voltage out from channel 3 )
  int powerDown = dac.getPowerDown(3); // get current Power-Down setting of channel 3
  Serial.print("PowerDown setting of channel 3 = "); // serial print of value
  Serial.println(powerDown, DEC); // serial print of value

              Write values to EEPROM
                Writing value to EEPROM always update input register as well.
                Writing to EEPROM normally take about 50ms.
                To write Vref, Gain, Power-Down settings to EEPROM, just use eepromWrite() after set them in input registers.

  dac.eepromWrite(1500,1500,1500,1500); // write to EEPROM of DAC four channel together. Value 0-4095
  delay(100);//writing to EEPROM takes about 50ms
  dac.eepromWrite(1, 1000); // write to EEPROM of DAC. Channel 0-3, Value 0-4095
  delay(100);//writing to EEPROM takes about 50ms
  dac.eepromWrite(); // write all input register values and settings to EEPROM
  delay(100);//

              Get Device ID (up to 8 devices can be used in a I2C bus, Device ID = 0-7)

  int id = dac.getId(); // return devideID of object
  Serial.print("Device ID  = "); // serial print of value
  Serial.println(id, DEC); // serial print of value
  
   * 
   */

