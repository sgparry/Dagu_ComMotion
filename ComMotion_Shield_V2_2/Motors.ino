void Motors()
{
  //============================================================================ Motor Speed Control ====================================================================================

  unsigned long actual;                                                       // temporary calculation of desired speed in uS per encoder pulse
  static byte astall,bstall;                                                  // flags to indicate a stalled motor
  

  if(aflag==1 || astall==1)                                                   // if encoder A has changed states or motor A has stalled                  
  {
    apulse=(micros()-atime);                                                  // time between last state change and this state change
    atime=micros();                                                           // update atime with time of most recent state change
        
    if(aflag)                                                                 // if encoder A has changed states
    {
      acount=acount+(mspeed[motora]>0)-(mspeed[motora]<0);                    // update encoder A state change counter
      if(encstop[motora]!=0)                                                  // if "stop on encoder count" is enabled
      {
        if(abs(encstart[motora]-acount)>=encstop[motora])                     // check if motor has travelled the desired distance
        {
          mspeed[motora]=0;                                                   // set desired speed to 0
          apwm=0;                                                             // set PWM to 0
          encstop[motora]=0;                                                  // reset encoder stop
          bitSet(encstopreport,motora);                                       // report to master that motor A has achieved position
        }
      }
    }
    actual=maxpulse[motora]/abs(mspeed[motora]);                              // calculate desired time in uS between encoder pulses
    if(actual>apulse && apwm>0) apwm--;                                       // if motor is running too fast then decrease PWM
    if(actual<apulse && apwm<255) apwm++;                                     // if motor is running too slow then increase PWM
    if(mspeed[motora]==0) apwm=0;                                             // override speed adjust if speed is 0 for quick stop
    analogWrite(pwmapin,apwm);                                                // update motor speed
    digitalWrite(dirapin,mspeed[motora]>0);                                   // set direction of motor
    astall=0;                                                                 // reset stall flag
    aflag=0;                                                                  // reset encoder flag
  }

  if(bflag==1 || bstall==1)                                                   // if encoder B has changed states or motor B has stalled  
  {  
    bpulse=(micros()-btime);                                                  // time between last state change and this state change
    btime=micros();                                                           // update btime with time of most recent state change
        
    if(bflag)                                                                 // if encoder B has changed states
    {
      bcount=bcount+(mspeed[motorb]>0)-(mspeed[motorb]<0);                    // update encoder B state change counter
      if(encstop[motorb]!=0)                                                  // if "stop on encoder count" is enabled
      {
        if(abs(encstart[motorb]-bcount)>=encstop[motorb])                     // check if motor has travelled the desired distance
        {
          mspeed[motorb]=0;                                                   // set desired speed to 0
          bpwm=0;                                                             // set PWM to 0
          encstop[motorb]=0;                                                  // reset encoder stop
          bitSet(encstopreport,motorb);                                       // report to master that motor B has achieved position 
        }
      }
    }
    actual=maxpulse[motorb]/abs(mspeed[motorb]);                              // calculate desired time in uS between encoder pulses
    if(actual>bpulse && bpwm>0) bpwm--;                                       // if motor is running too fast then decrease PWM
    if(actual<bpulse && bpwm<255) bpwm++;                                     // if motor is running too slow then increase PWM
    if(mspeed[motorb]==0) bpwm=0;                                             // override speed adjust if speed is 0 for quick stop
    analogWrite(pwmbpin,bpwm);                                                // update motor speed
    digitalWrite(dirbpin,mspeed[motorb]>0);                                   // set direction of motor
    bstall=0;                                                                 // reset stall flag
    bflag=0;                                                                  // reset encoder flag
  }  
  
  

  //============================================================================ Check for stalled motors ===============================================================================

  if(analogvalue[0]>maxamps[motora])                                          // motor A maximum current exceeded
  {
    apwm=apwm/2;                                                              // halve pwm
    if(mcu==0) eflag=eflag|B00000001;                                         // bit 0 indicates M1 has exceeded current limit
    if(mcu==1) eflag=eflag|B00000100;                                         // bit 2 indicates M3 has exceeded current limit
  }

  if(mspeed[motora]==0)                                                       // if motor A speed is supposed to be 0 or motor current exceeds limit
  {
    apwm=0;                                                                   // ensure apwm=0
    analogWrite(pwmapin,apwm);                                                // cut power to motor A
  }
  else
  {
    if(micros()-atime>(stalltm[motora]*1000L))                                // if encoder A has not changed states within 10mS
    {
      astall=1;                                                               // set motor A stall flag
      apwm+=2;                                                                // jump start apwm value
      if(apwm>253) apwm=253;
    }
  }

  if(analogvalue[1]>maxamps[motorb])                                          // motor B maximum current exceeded
  {
    bpwm=bpwm/2;                                                              // halve pwm
    if(mcu==0) eflag=eflag|B00000010;                                         // bit 1 indicates M2 has exceeded current limit
    if(mcu==1) eflag=eflag|B00001000;                                         // bit 3 indicates M4 has exceeded current limit
  }

  if(mspeed[motorb]==0)                                                       // if motor B speed is supposed to be 0 or motor current exceeds limit
  {
    bpwm=0;                                                                   // ensure bpwm=0
    analogWrite(pwmbpin,bpwm);                                                // cut power to motor B
  }
  else
  {
    if(micros()-btime>(stalltm[motorb]*1000L))                                // if encoder B has not changed states within 10mS 
    {
      bstall=1;                                                               // set motor B stall flag
      bpwm+=2;                                                                // jump start bpwm value
      if(bpwm>253) bpwm=253;
    }
  }
  if(encstopreport>0)
  {
    Wire.beginTransmission(master);
    Wire.write(encstopreport);
    encstopreport=0;
    Wire.endTransmission();
  }
}


//============================================================================== Encoder ISRs =================================================================

void Aencoder()                                                               // left  encoder Interrupt Service Routine
{
  aflag=1;                                                                    // set flag for left  encoder
}

void Bencoder()                                                               // right encoder Interrupt Service Routine
{
  bflag=1;                                                                    // set flag for right encoder
}


