
//============================================================== Receive I²C command from external device or other MCU =======================

void I2C_Receive(int bytes)                                   // received command as I²C slave     
{
  for(byte i=0;i<bytes;i++)                                 
  {
    datapack[i]=Wire.read();                                  // transfer data from I²C buffer to datapack
  }
  
  command=datapack[0];
  packsize=bytes;
  
  //Serial.print("Pack size:");Serial.println(bytes,DEC);
  //Serial.print("Command:");Serial.println(command,DEC);
  
  if(mode==DCBCM_DEMO && command<DCC_SERIAL_SEND)
  {
    mode=DCBCM_SERIAL_I2C;                                    // disable demo mode
    EEPROM.write(1,mode);
  }
  
    
  //------------------------------------------------------------ demo syncronization ---------------------------------------------------------
  
  if(command==DCC_DEMO_SYNC && packsize==3)
  {
    EEPROM.write(0,datapack[1]);                              // only update EEPROM if necessary
    EEPROM.write(1,datapack[2]);                              // only update EEPROM if necessary
    
    demo=datapack[1];
    mode=datapack[2];
    
    command=DCC_NONE;
    return;
  }
  
  if(command==DCC_DEMO_ANGLE_UPDATE && packsize==3)  //============================= Demo Modes Angle Update ======================================================
  {
    angle=datapack[1]*256+datapack[2];
    Trigonometry();
    command=DCC_NONE;
    return;
  }
}
  
  
