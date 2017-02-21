#include <avr/pgmspace.h>

#define FS(x) (__FlashStringHelper*)(x)

const char STR_M[] PROGMEM =  "M";

void TXconfig()
{
  if(mcu)
  {
    //Serial.begin(57600);
    
    Serial.println();
    Serial.print(F("Demo:"));Serial.println(demo,DEC);
    Serial.print(F("Mode:"));Serial.println(mode,DEC);
    Serial.print(F("Configuration:"));Serial.println(configuration,DEC);
    Serial.println();
    Serial.print(F("Lo Bat:"));Serial.print(lowbat/10.0);Serial.println(F("V"));
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" maximum current:"));Serial.print(maxamps[i]/100.0);Serial.println(F("A"));
    }
    Serial.println();
    Serial.print(F("Master I2C address:"));Serial.println(master,DEC);
    Serial.print(F("I2C address offset:"));Serial.println(addroffset,DEC);
    Serial.print(F("I2C clock frequency:"));Serial.print(i2cfreq*300+100);Serial.println(F("Kb/sec"));
    Serial.println();
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" maximum speed:"));Serial.print(motrpm[i]); Serial.println(F("RPM"));
    }
    Serial.println();
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" encoder resolution:"));Serial.println(encres[i]/100.0);
    }
    Serial.println();
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" maximum encoder pulse:"));Serial.print(maxpulse[i]/255L);Serial.println(F("uS"));
    }
    Serial.println();
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" reserve power:"));Serial.print(reserve[i],DEC);Serial.println(F("%"));
    }
    Serial.println();
    for(int i=0; i<4; i++)
    {
      Serial.print(FS(STR_M));Serial.print(i);Serial.print(F(" stall time:"));Serial.print(stalltm[i]);Serial.println(F("mS"));
    }
    Serial.println();
    Serial.print(F("Serial mode:"));Serial.println(sermode,DEC);
    Serial.print(F("Baud rate port 1:"));Serial.println(baudrate[0]);
    Serial.print(F("Baud rate port 2:"));Serial.println(baudrate[1]);
    Serial.print(F("Defaults:"));Serial.println(EEPROM.read(40),DEC);
    Serial.println();
    Serial.println();
  }
}

