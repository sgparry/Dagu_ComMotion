#include <Dagu_ComMotionCommon.h>

void Commands()
{
  if(mcu==0 && (command<DCC_SERIAL_SEND || command==DCC_EEROM_DEFAULTS))     // if this is MCU1 and command is 0-4 then repeat command to MCU2 
  {
    Wire.beginTransmission(address+1);                                       // take control of I²C bus and address MCU2
    Wire.write(datapack,packsize);                                           // relay commands to MCU2
    Wire.endTransmission();                                   
  }
  
  if(command==DCC_EEROM_DEFAULTS) 
  {
    EEPROMdefaults();
    command=DCC_NONE;
    return;
  }
  
  if(command>DCC_DEMO_SYNC) command-=DCC_INTERNAL;                                // 16 is added (bit 3 set high) for internal commands (no repeat)
  
  
  
  
  if(command==DCC_BASIC_CONFIG && packsize==sizeof(Dagu_ComMotionBasicConfigPacket)) //============================================ Basic Configuration Data Received =================================================
  {
    mode=datapack[1];
    configuration=datapack[2];
    lowbat=datapack[3];
    maxamps[0]=datapack[4];
    maxamps[1]=datapack[5];
    maxamps[2]=datapack[6];
    maxamps[3]=datapack[7];
    addroffset=datapack[8];
    master=datapack[9];
    EEPROMsave();                                                            // update EEPROM
    TXconfig();
    command=DCC_NONE;
    return;
  }

  if(command==DCC_ENCODER_CONFIG) //============================================================ Encoder Configuration Data Received =================================================
  {
    if(packsize == sizeof(Dagu_ComMotionEncoderConfigPacket4) || packsize == sizeof(Dagu_ComMotionEncoderConfigPacketX4))                                         // configure each encoder individually
    {
      for(byte i=0;i<4;i++)
      {
        motrpm[i]=datapack[i*2+1]*256+datapack[i*2+2];
        encres[i]=datapack[i*2+9]*256+datapack[i*2+10];
        reserve[i]=datapack[17+i];
        stalltm[i]=datapack[21+i];
        if(packsize==sizeof(Dagu_ComMotionEncoderConfigPacketX4))
          encflags[i]=datapack[25+i];
      }
    }
    else if(packsize == sizeof(Dagu_ComMotionEncoderConfigPacket1) || packsize == sizeof(Dagu_ComMotionEncoderConfigPacketX1))                                      // use 1 configuration for all encoders
    {
      for(byte i=0;i<4;i++)
      {
        motrpm[i]=datapack[1]*256+datapack[2];
        encres[i]=datapack[3]*256+datapack[4];
        reserve[i]=datapack[5];
        stalltm[i]=datapack[6];
        if(packsize==sizeof(Dagu_ComMotionEncoderConfigPacketX4))
            encflags[i]=datapack[7];
      }
    }
    for(byte i=0;i<4;i++)
    {
      maxpulse[i]=60000000L/(long(motrpm[i])*long(encres[i])/100L)*255L;     // update maxpulse values
      maxpulse[i]=maxpulse[i]*(100L-long(reserve[i]))/100L;
    }
    EEPROMsave();                                                            // update EEPROM
    TXconfig();
    command=DCC_NONE;
    return;
  }

  if(command==DCC_MOTOR_CONTROL && (packsize == sizeof(Dagu_ComMotionOmniMotorControlPacket) || packsize == sizeof(Dagu_ComMotionIndeMotorControlPacket))) //============================ Motor Control =======================================================================
  {
    if((configuration==DCC_INDEPENDENT || configuration==DCC_INDEPENDENT+DCC_NO_ENCODERS) && packsize==sizeof(Dagu_ComMotionIndeMotorControlPacket))               // Individual motor control  
    {
      for(byte i=0;i<4;i++)
      {
        mspeed[i]=datapack[i*2+1]*256+datapack[i*2+2];
      }
    }
    else                                                                     // Omni or Mecanum Wheels 
    {
      velocity=datapack[1]*256+datapack[2];
      angle=datapack[3]*256+datapack[4];
      rotation=datapack[5]*256+datapack[6];
      
      if(velocity>255) velocity=255;
      if(velocity<-255) velocity=-255;
      
      while(angle>=360) 
      {
        angle-=360;
      }
      while(angle<0) 
      {
        angle+=360;
      }
      
      if(rotation>255) rotation=255;
      if(rotation<-255) rotation=-255;
      
      Trigonometry();
    }
    command=DCC_NONE;  
    return;
  }

  if(command==DCC_SERIAL_CONFIG && packsize == sizeof(Dagu_ComMotionSerialConfigPacket)) //============================================= Serial port configuration ===========================================================
  {
    baudrate[0]=datapack[1]*256U+datapack[2];
    baudrate[1]=datapack[3]*256U+datapack[4];
    sermode=datapack[5];
    Serial.begin(baudrate[mcu]);                                             // change serial port baud rate
    EEPROMsave();                                                            // update EEPROM
    command=DCC_NONE;
    return;
  }

  if(command==DCC_SERIAL_SEND && packsize>2 && packsize<33) //=============================== Send Serial Data =====================================================================
  {
    if((mcu+1)==datapack[1])
    {
      for(byte i=0;i<(packsize-2);i++)
      {
        serpack[i]=datapack[i+2];
      }
      Serial.write(serpack,packsize-2);
    }
    command=DCC_NONE;
    return;
  }  

  if(command==DCC_STATUS_REQUEST && packsize == sizeof(Dagu_ComMotionStatusRequestPacket)) //============================================= Status request ======================================================================
  {
    // Status Command Bit Meanings
    // ===========================
    // Note: revised for 2.3.1
    // Only one MCU's status is returned at a time - a command must be sent to each.
    // Bit 0: Returns 8 bytes, the encoder count from each motor high byte first.
    // Bit 1: Resets all encoder counters. If bit 0 is high then the counters will be read before being reset.
    // Bit 2: Returns 8 bytes, the current draw of each motor high byte first.
    // Bit 3: If sent to MCU1: Returns 6 bytes, the analog inputs A3, A6 and A7 from MCU1, high byte first. A7 is battery voltage*.
    //        If sent to MCU2: Returns 1 byte MCU number **,
    // Bit 4: If sent to MCU1: Returns 1 byte MCU number **,
    //        If sent to MCU2: Returns 6 bytes, the analog inputs A3, A6 and A7 from MCU1, high byte first.
    // Bit 5: Returns 1 byte, the error log for the motors.
    // Bit 6: Clears the error logs. If bit 5 or 6 are high then those error logs will be read first.
    // Bit 7: Used to indicate internal communication - return the data to the other MCU, rather than the host MCU.
    //
    // * Battery voltage = MCU1analog input A7 * 30 / 185. A result of 84 = 8.4V
    // ** The MCU number is added as the _first_ byte: 1 = MCU1, 2 = MCU2. Can be used to easily identify which MCU originated the packet.

                                                                             // each mcu sends it's data seperately to minimize interferance with motor speed control
                                                                             
    byte spsize=0;                                                           // intitial send pack size = 0
    int request=datapack[1];                                                 // copy datapack to global variable "request" ASAP so datapack can be reused

    if(((request & Dagu_ComMotionStatusRequestPacket::DR_MCU_NUM_MCU1) && mcu==0) ||
      ((request & Dagu_ComMotionStatusRequestPacket::DR_MCU_NUM_MCU2) && mcu==1))                               // start the packet with the MCU number, if requested.
    {
      sendpack[spsize+0] = mcu + 1;
      spsize++;
    }
    
    if(request & Dagu_ComMotionStatusRequestPacket::DR_ENCODERS)// Bit 0:                                       // return encoder counter values
    {
      sendpack[spsize+0] =highByte(acount);
      sendpack[spsize+1] = lowByte(acount);
      sendpack[spsize+2] =highByte(bcount);
      sendpack[spsize+3] = lowByte(bcount);
      spsize+=4;                                                                                                // increment pack size by 4 bytes (counts from 2 encoders only)
    }

    if(request & Dagu_ComMotionStatusRequestPacket::DR_ENCODERS_RESET)// Bit 1:                                 // reset encoder counters
    {
      acount=0;
      bcount=0;
    }

    if(request & Dagu_ComMotionStatusRequestPacket::DR_CURRENT_DRAW)// Bit 2:                                   // return motor currents
    {
      sendpack[spsize+0] =highByte(analogvalue[0]);
      sendpack[spsize+1] = lowByte(analogvalue[0]);
      sendpack[spsize+2] =highByte(analogvalue[1]);
      sendpack[spsize+3] = lowByte(analogvalue[1]);
      spsize+=4;                                                                                                // increment pack size by 4 bytes (current from 2 motors only)
    }

    if(((request & Dagu_ComMotionStatusRequestPacket::DR_ANALOG_VALS_MCU1) && mcu==0) ||
      ((request & Dagu_ComMotionStatusRequestPacket::DR_ANALOG_VALS_MCU2) && mcu==1))// Bits 3&4:               // return MCU analog values
    {
      sendpack[spsize+0]=highByte(analogvalue[2]);
      sendpack[spsize+1]= lowByte(analogvalue[2]);
      sendpack[spsize+2]=highByte(analogvalue[3]);
      sendpack[spsize+3]= lowByte(analogvalue[3]);
      sendpack[spsize+4]=highByte(analogvalue[4]);
      sendpack[spsize+5]= lowByte(analogvalue[4]);
      spsize+=6;
    }

    if(request & Dagu_ComMotionStatusRequestPacket::DR_MOTOR_ERR_LOG)// Bit 5:                                  // return error log
    {
      sendpack[spsize++]=eflag;
    }

    if(request & Dagu_ComMotionStatusRequestPacket::DR_CLEAR_MOTOR_ERR_LOG)// Bit 6:                            // clear error log
    {  
      eflag=0;
    }
    
    byte returnaddress=master;                                               // return address is I²C master by default
    if((request & Dagu_ComMotionStatusRequestPacket::DR_INTERNAL) && mcu==0) returnaddress=address+1;           // bit 7 indicates internal request - return to other processor
    if((request & Dagu_ComMotionStatusRequestPacket::DR_INTERNAL) && mcu==1) returnaddress=address-1;           // bit 7 indicates internal request - return to other processor
    
    Wire.beginTransmission(returnaddress);
    Wire.write(sendpack,spsize);
    Wire.endTransmission();
    command=DCC_NONE;
  }
}
