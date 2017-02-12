void IRcommands()
{
  if(millis()-IRtime>50) 
  {
    IRtime=millis();
    
    if(IRC==1) angle=315;
    if(IRC==2) angle=0;
    if(IRC==3) angle=45;
    if(IRC==4) angle=270;
    if(IRC==6) angle=90;
    if(IRC==7) angle=225;
    if(IRC==8) angle=180;
    if(IRC==9) angle=135;
    
    if(IRC==17) velocity+=5;
    if(IRC==18) velocity-=5;
    if(IRC==19) rotation-=5;
    if(IRC==20) rotation+=5;
    
    if(IRC==38 || IRC==5)
    {
      velocity=0;
      angle=0;
      rotation=0;
    }
    
            
    if(velocity>255) velocity=255;
    if(velocity<-255) velocity=-255;
    if(rotation>255) rotation=255;
    if(rotation<-255) rotation=-255;
    if(angle>=360) angle=angle-360;
    if(angle<0) angle+=360;
    
    Wire.beginTransmission(address-1);
    Wire.write(3);                                       
    Wire.write(highByte(velocity));
    Wire.write( lowByte(velocity));
    Wire.write(highByte(angle));
    Wire.write( lowByte(angle));
    Wire.write(highByte(rotation));
    Wire.write( lowByte(rotation));
    Wire.endTransmission();
  }
  else
  {
    IRC=0;                                                                    // prevent additional IR commands from being recognised for 50mS after initial IR command
  }
}
