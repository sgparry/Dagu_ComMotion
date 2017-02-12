void IRdecode()
{
  //----------------------------------------------------------- Sony IRC Decoder ------------------------------------------------------
  
  volatile static byte irdata;                               // temporary storage of byte value as bits are read
  volatile static byte irbitvalue;                           // decimal value of bit being read (1,2,4,8,16,32,64,128)
  volatile static byte irbitcount;                           // counts number of bits since start bit (only first 7 bits are used)
  volatile static byte irflag;                               // bit width in 200uS chunks
  volatile static byte newbit;
  
    
  if(digitalRead(irpin)==0)                                  // check state of IR receiver output 
  {
    irflag++;                                                // increment irflag used to measure ir pulse widths
    newbit=1;                                                // recognizes a new bit being measured
  }
  else                                                       // IR receiver output high - end of bit
  {
    if(newbit==1)
    {
      if(irflag>4)                                           // bitwidth>4 = logic "1"
      {
        irdata+=irbitvalue;                                  // increment data by bit value
      }
      irbitvalue*=2;                                         // bitvalue updated for next bit (1,2,4,8,16,32,64,128)
      irbitcount++;                                          // count the bits

      if(irbitcount>6)                                       // only want first 7 bits
      {
        irbitvalue=0;                                        // additional bits have a value of 0
        IRC=irdata+1;                                        // make data available to user
      }

      if(irflag>8)                                           // SIRC start bit resets decoder values
      {
        irbitvalue=1;
        irbitcount=0;
        irdata=0;
      }

      newbit=0;                                              // addional interrupts not new bit
    }
    irflag=0;                                                // reset irflag when IR pulse ends  
  }
}
