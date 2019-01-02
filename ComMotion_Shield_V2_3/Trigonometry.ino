void Trigonometry()
{
  if(configuration==DCC_3XOMNI)                                                               // 3 Omni wheels
  {
    if(mcu==0)
    {
      mspeed[0]=int(sin((angle+120.0)*radconvert)*float(velocity))+rotation;                  // calculate desired speed
      mspeed[1]=int(sin((angle)*radconvert)*float(velocity))+rotation;                        // calculate desired speed
    }
    else
    {
      mspeed[2]=int(sin((angle-120.0)*radconvert)*float(velocity))+rotation;                  // calculate desired speed
      mspeed[3]=0;                                                                            // 4th motor not used
    }
    return;
  }
  
  if(configuration==DCC_4XOMNI)                                                               // 4 Omni wheels
  {
    if(mcu==0)
    {
      mspeed[0]=int(sin((angle)*radconvert)*velocity)+rotation;                               // calculate desired speed
      mspeed[1]=int(sin((angle+90)*radconvert)*velocity)+rotation;                            // calculate desired speed
    }
    else
    {
      mspeed[2]=int(sin((angle+180)*radconvert)*velocity)+rotation;                           // calculate desired speed
      mspeed[3]=int(sin((angle+270)*radconvert)*velocity)+rotation;                           // calculate desired speed
    }
    return;
  }
  
  if(configuration==DCC_MECANUM)                                                              // Mecanum Wheels
  {
    if(mcu==0)
    {
      mspeed[0]=int((-sin(angle*radconvert)+cos(angle*radconvert))*float(velocity))+rotation; // calculate desired speed
      mspeed[1]=int((+sin(angle*radconvert)+cos(angle*radconvert))*float(velocity))+rotation; // calculate desired speed
    }
    else
    {
      mspeed[2]=int((-sin(angle*radconvert)+cos(angle*radconvert))*float(velocity))-rotation; // calculate desired speed
      mspeed[3]=int((+sin(angle*radconvert)+cos(angle*radconvert))*float(velocity))-rotation; // calculate desired speed
    }
    return;
  }
}
