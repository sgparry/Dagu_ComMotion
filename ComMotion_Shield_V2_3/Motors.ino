void Motors()
{
  //============================================================================ Motor Speed Control ====================================================================================

  unsigned long actual;                                                       // temporary calculation of desired speed in uS per encoder pulse
  static byte apwm,bpwm;                                                      // A and B motor speeds
  static byte astall,bstall;                                                  // flags to indicate a stalled motor

  byte oslider = slider;
  slider ^= 1;                                                                // switch slots so that the isr can continue without clashing.
  byte inc = ainc[oslider];
  if(inc>0 || astall==1)                                                   // if encoder A has changed states or motor A has stalled                  
  {
    apulse=(micros()-atime);                                                  // time between last state change and this state change
    atime=micros();                                                           // update atime with time of most recent state change
        
    if(inc > 0)
    {
      if(mspeed[motora]>0 || (encflags[motora] & 0x1))                        // update encoder A state change counter
        acount += inc;
      else if (mspeed[motora]<0)
        acount -= inc;
    }
    actual=maxpulse[motora]/abs(mspeed[motora]);                              // calculate desired time in uS between encoder pulses
    if(actual>apulse && apwm>0) apwm--;                                       // if motor is running too fast then decrease PWM
    if(actual<apulse && apwm<255) apwm++;                                     // if motor is running too slow then increase PWM
    if(mspeed[motora]==0) apwm=0;                                             // override speed adjust if speed is 0 for quick stop
    analogWrite(pwmapin,apwm);                                                // update motor speed
    digitalWrite(dirapin,mspeed[motora]>0);                                   // set direction of motor
    astall=0;                                                                 // reset stall flag
    ainc[oslider]=0;                                                          // reset encoder temp count
  }

  inc = binc[oslider];
  if(inc>0 || bstall==1)                                                   // if encoder B has changed states or motor B has stalled  
  {  
    bpulse=(micros()-btime);                                                  // time between last state change and this state change
    btime=micros();                                                           // update btime with time of most recent state change
        
    if(inc > 0)
    {
      if(mspeed[motorb]>0 || (encflags[motorb] & 0x1))                        // update encoder B state change counter
        bcount += inc;
      else if (mspeed[motorb]<0)
        bcount -= inc;
    }
    actual=maxpulse[motorb]/abs(mspeed[motorb]);                              // calculate desired time in uS between encoder pulses
    if(actual>bpulse && bpwm>0) bpwm--;                                       // if motor is running too fast then decrease PWM
    if(actual<bpulse && bpwm<255) bpwm++;                                     // if motor is running too slow then increase PWM
    if(mspeed[motorb]==0) bpwm=0;                                             // override speed adjust if speed is 0 for quick stop
    analogWrite(pwmbpin,bpwm);                                                // update motor speed
    digitalWrite(dirbpin,mspeed[motorb]>0);                                   // set direction of motor
    bstall=0;                                                                 // reset stall flag
    binc[oslider]=0;                                                          // reset encoder temp count
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
}


//============================================================================== Encoder ISRs =================================================================

void Aencoder()                                                               // left  encoder Interrupt Service Routine
{
  ainc[slider] ++;                                                                    // increment holding counter for A
}

void Bencoder()                                                               // right encoder Interrupt Service Routine
{
  binc[slider] ++;                                                                    // increment holding counter B
}


