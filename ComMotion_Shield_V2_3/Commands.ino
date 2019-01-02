#define dword(h,l) ((h)<<16 + (l))
#define lowWord(dw) ((uint16_t) ((dw) & 0xffff))
#define highWord(dw) ((uint16_t) ((dw) >> 16))

void Commands()
{
  if(mcu==0 && (command<5 || command==10))                                   // if this is MCU1 and command is 0-4 then repeat command to MCU2 
  {
    Wire.beginTransmission(address+1);                                       // take control of I²C bus and address MCU2
    Wire.write(datapack,packsize);                                           // relay commands to MCU2
    Wire.endTransmission();                                   
  }
  
  if(command==10) 
  {
    EEPROMdefaults();
    command=255;
    return;
  }
  
  if(command>15) command-=16;                                                // 16 is added (bit 3 set high) for internal commands (no repeat)
  
  
  
  
  if(command==1 && packsize==10) //============================================ Basic Configuration Data Received =================================================
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
    command=255;
    return;
  }

  if(command==2) //============================================================ Encoder Configuration Data Received =================================================
  {
    if(packsize==2) // just extended flags, all encoders
    {
      for(byte i=0;i<4;i++)
      {
        encflags[i]=datapack[1];
      }
    }
    else if(packsize==5) // just extended flags, individually
    {
      for(byte i=0;i<4;i++)
      {
        encflags[i]=datapack[1+i];
      }
    }
    if(packsize==25 || packsize==29)                                         // configure each encoder individually
    {
      for(byte i=0;i<4;i++)
      {
        motrpm[i]=datapack[i*2+1]*256+datapack[i*2+2];
        encres[i]=datapack[i*2+9]*256+datapack[i*2+10];
        reserve[i]=datapack[17+i];
        stalltm[i]=datapack[21+i];
        if(packsize==29)
          encflags[i]=datapack[25+i];
      }
    }
    else if(packsize==7 || packsize==8)                                      // use 1 configuration for all encoders
    {
      for(byte i=0;i<4;i++)
      {
        motrpm[i]=datapack[1]*256+datapack[2];
        encres[i]=datapack[3]*256+datapack[4];
        reserve[i]=datapack[5];
        stalltm[i]=datapack[6];
        if(packsize==8)
            encflags[i]=datapack[7];
      }
    }
    for(byte i=0;i<4;i++)
    {
      maxpulse[i] = (long) ( (60LL * 1000000LL * 100LL * 100LL * 255LL) / (int64_t(motrpm[i])*int64_t(encres[i]) * (100LL-int64_t(reserve[i]))) );
                                                                             // update maxpulse
                                                                             // 60 * 1000000 - convert from rpm to period in uS
                                                                             // 255 = max motor pwm, will divide later for smaller values
                                                                             // 100 = encRes divisor, 100 = reserve percentage divisor
                                                                             // 100-reserve = precentage after reserve
                                                                             // motrpm = motor revs per minute
                                                                             // encres = encoder resolution in 100ths of a segment per revolution
    }
    EEPROMsave();                                                            // update EEPROM
    TXconfig();
    command=255;
    return;
  }

  if(command==3 && (packsize==7 || packsize==9)) //============================ Motor Control =======================================================================
  {
    if((configuration==3 || configuration==19) && packsize==9)               // Individual motor control  
    {
      for(byte i=0;i<4;i++)
      {
        mspeed[i]=datapack[i*2+1]*256+datapack[i*2+2];
        apwm = abs(mspeed[motora]);
        bpwm = abs(mspeed[motorb]);
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
    command=255;  
    return;
  }

  if(command==4 && packsize==6) //============================================= Serial port configuration ===========================================================
  {
    baudrate[0]=datapack[1]*256U+datapack[2];
    baudrate[1]=datapack[3]*256U+datapack[4];
    sermode=datapack[5];
    Serial.begin(baudrate[mcu]);                                             // change serial port baud rate
    EEPROMsave();                                                            // update EEPROM
    command=255;
    return;
  }

  if(command==5 && packsize>2 && packsize<33) //=============================== Send Serial Data =====================================================================
  {
    if((mcu+1)==datapack[1])
    {
      for(byte i=0;i<(packsize-2);i++)
      {
        serpack[i]=datapack[i+2];
      }
      Serial.write(serpack,packsize-2);
    }
    command=255;
    return;
  }  

  if(command==6 && (packsize==2 || packsize==3)) //============================================= Status request ======================================================================
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

    if(((request&16) && mcu==0) || ((request&8) && mcu==1))                 // start the packet with the MCU number, if requested.
    {
      sendpack[spsize+0] = mcu + 1;
      spsize++;
    }
    
    if(request&1)// Bit 0:                                                   // return encoder counter values
    {
      sendpack[spsize+0] =highByte(acount);
      sendpack[spsize+1] = lowByte(acount);
      sendpack[spsize+2] =highByte(bcount);
      sendpack[spsize+3] = lowByte(bcount);
      spsize+=4;                                                              // increment pack size by 4 bytes (counts from 2 encoders only)
    }

    if(request&2)// Bit 1:                                                   // reset encoder counters
    {
      acount=0;
      bcount=0;
    }

    if(request&4)// Bit 2:                                                   // return motor currents
    {
      sendpack[spsize+0] =highByte(analogvalue[0]);
      sendpack[spsize+1] = lowByte(analogvalue[0]);
      sendpack[spsize+2] =highByte(analogvalue[1]);
      sendpack[spsize+3] = lowByte(analogvalue[1]);
      spsize+=4;                                                             // increment pack size by 4 bytes (current from 2 motors only)
    }

    if(((request&8) && mcu==0) || ((request&16) && mcu==1))// Bits 3&4:      // return MCU analog values
    {
      sendpack[spsize+0]=highByte(analogvalue[2]);
      sendpack[spsize+1]= lowByte(analogvalue[2]);
      sendpack[spsize+2]=highByte(analogvalue[3]);
      sendpack[spsize+3]= lowByte(analogvalue[3]);
      sendpack[spsize+4]=highByte(analogvalue[4]);
      sendpack[spsize+5]= lowByte(analogvalue[4]);
      spsize+=6;
    }

    if(request&32)// Bit 5:                                                  // return error log
    {
      sendpack[spsize++]=eflag;
    }

    if(request&64)// Bit 6:                                                  // clear error log
    {  
      eflag=0;
    }

    if(packsize==3)
    {
      int request2=datapack[2];
      if(request2 & 1)
      {
        for(int i = 0; i < 2; i++)
        {
          sendpack[spsize+0]=highByte(highWord(maxpulse[i]));
          sendpack[spsize+1]= lowByte(highWord(maxpulse[i]));
          sendpack[spsize+2]=highByte( lowWord(maxpulse[i]));
          sendpack[spsize+3]= lowByte( lowWord(maxpulse[i]));
          spsize+=4;
        }
      }
      if(request2 & 2)
      {
        sendpack[spsize+0]=highByte(highWord(apulse));
        sendpack[spsize+1]= lowByte(highWord(apulse));
        sendpack[spsize+2]=highByte( lowWord(apulse));
        sendpack[spsize+3]= lowByte( lowWord(apulse));
        spsize+=4;
        sendpack[spsize+0]=highByte(highWord(bpulse));
        sendpack[spsize+1]= lowByte(highWord(bpulse));
        sendpack[spsize+2]=highByte( lowWord(bpulse));
        sendpack[spsize+3]= lowByte( lowWord(bpulse));
        spsize+=4;
      }
      if(request2 & 4)
      {
        sendpack[spsize+0]= apwm;
        sendpack[spsize+1]= bpwm;
        spsize+=2;
      }
      if(request2 & 8)
      {
        sendpack[spsize+0]= astall;
        sendpack[spsize+1]= bstall;
        spsize+=2;
      }
    }
    
    byte returnaddress=master;                                               // return address is I²C master by default
    if((request&128) && mcu==0) returnaddress=address+1;                     // bit 7 indicates internal request - return to other processor
    if((request&128) && mcu==1) returnaddress=address-1;                     // bit 7 indicates internal request - return to other processor

    Wire.beginTransmission(returnaddress);
    Wire.write(sendpack,spsize);
    Wire.endTransmission();
    command=255;
  }
  if(command==11 && packsize==2)
  {
    Beep(datapack[1]);
    command=255;
  }
  if(command==12 && packsize>=2)
  {
    byte returnaddress=datapack[1];
    Wire.beginTransmission(returnaddress);
    Wire.write(address);
    Wire.write(mcu);
    Wire.write(&datapack[2],packsize-2);
    Wire.endTransmission();
    command=255;
  }
}
