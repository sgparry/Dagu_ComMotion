//======================================================================================== Takes data from serial port and redirects it as determined by serial mode =======================================
void SerialInput()                                                        
{
  if((mcu==0 && (sermode==DCSM_COMMANDS_ON_PORT1_DATA_TO_MASTER || sermode==DCSM_COMMANDS_ON_PORT1_DATA_TO_PORT1)) || 
    (mcu==1 && (sermode==DCSM_COMMANDS_ON_PORT2_DATA_TO_MASTER || sermode==DCSM_COMMANDS_ON_PORT2_DATA_TO_PORT2)))  // Command received on serial port
  {
    byte i=0;
    command=DCC_NONE;
    while(Serial.available()>0)
    {
      datapack[i]=Serial.read();                                                        // data is read from the serial buffer and added to the command datapack
      i++;
    }
    
    //------------------------------------------------------------------------------------ Verify that command data has been received ---------------------------------------------------------------------
    if(datapack[0]==DCC_BASIC_CONFIG && i==sizeof(Dagu_ComMotionBasicConfigPacket))
    {
      command=datapack[0];
      packsize=i;
    }
    
    if(datapack[0]==DCC_ENCODER_CONFIG && (i == sizeof(Dagu_ComMotionEncoderConfigFlagsPacket1) || i == sizeof(Dagu_ComMotionEncoderConfigFlagsPacket4) || i == sizeof(Dagu_ComMotionEncoderConfigPacketX1) || i == sizeof(Dagu_ComMotionEncoderConfigPacketX4) || i==sizeof(Dagu_ComMotionEncoderConfigPacket1) || i==sizeof(Dagu_ComMotionEncoderConfigPacket4)))
    {
      command=datapack[0];
      packsize=i;
    }
    
    if(datapack[0]==DCC_MOTOR_CONTROL && (i==sizeof(Dagu_ComMotionOmniMotorControlPacket) || i==sizeof(Dagu_ComMotionIndeMotorControlPacket)))
    {
      command=datapack[0];
      packsize=i;
    }
    
    if(datapack[0]==DCC_SERIAL_CONFIG && i==sizeof(Dagu_ComMotionSerialConfigPacket))
    {
      command=datapack[0];
      packsize=i;
    }
    
    if(datapack[0]==DCC_SERIAL_SEND && i>2)
    {
      command=datapack[0];
      packsize=i;
    }
    
    if(command==DCC_NONE)
    {
      Serial.println(F("Command not recognized!"));
      Serial.print(F("Command:"));Serial.print(datapack[0],DEC);
      Serial.print(F("Pack Size:"));Serial.println(i,DEC);
      Serial.println();
    }
    else
    {
      if(mode==DCBCM_DEMO)
      {
        mode=DCBCM_SERIAL_I2C;                                                          // disable demo mode
        EEPROM.write(1,mode);
      }
    }
  }
  else
  {
    byte j=4;                                                                           // byte 1-4 is serial pack header
    serpack[0]='S';                                                                     // this header is used to indicate
    serpack[1]='P';                                                                     // the start of a new data packet
    serpack[2]=mcu+49;                                                                  // and where the data came from
    serpack[3]=':';                                                                     // "SP1:" or "SP2:" is serial port 1 or 2
    while(Serial.available()>0)
    {
      serpack[j]=Serial.read();                                                         // data is read from the serial buffer and added to the datapack
      j++;
    }
    
    if (sermode==DCSM_DATA_TO_MASTER ||
      (sermode==DCSM_COMMANDS_ON_PORT1_DATA_TO_MASTER && mcu==1) ||
      (sermode==DCSM_COMMANDS_ON_PORT2_DATA_TO_MASTER && mcu==0))                       // send serial data to I²C master
    {
      Wire.beginTransmission(master);                                                   // address I²C master
      Wire.write(serpack,j);                                                            // send data with 4 byte header
      Wire.endTransmission();                                                           // release I²C bus
    }
    else if ((sermode==DCSM_COMMANDS_ON_PORT1_DATA_TO_PORT1 && mcu==1) ||
      (sermode==DCSM_COMMANDS_ON_PORT2_DATA_TO_PORT2 && mcu==0))                        // pass serial data to other serial port
    {
      byte pass=master;
      if(mcu==0) pass=address+1;                                                        // address of MCU to pass data to
      if(mcu==1) pass=address-1;
      Wire.beginTransmission(pass);                                                     
      Wire.write(DCC_SERIAL_CONFIG + DCC_INTERNAL);                                     // command to internal send serial data (4+16)
      Wire.write(j);                                                                    // serial pack size including header data
      Wire.write(serpack,j);                                                            // send serialpack
      Wire.endTransmission();
    }
  }
  
  
  
  
  
}
