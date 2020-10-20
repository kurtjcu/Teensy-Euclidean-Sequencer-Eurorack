#include <SD.h>
#include <U8g2lib.h>
#include <Wire.h>

File myFile;

const int chipSelect = BUILTIN_SDCARD;

int memSlot=0; //used for rotary selection of memory slot allocation
int memSlotMAX=32;

//-------------------for ssd1306 display--------------//
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 19, /* data=*/ 18);   
//U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);    

// HW I2C with pin remapping
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping
//U8G2_SSD1306_128X64_NONAME_F_6800 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_8080 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
//U8G2_SSD1306_128X64_VCOMH0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // same as the NONAME variant, but maximizes setContrast() range
//U8G2_SSD1306_128X64_ALT0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // same as the NONAME variant, but may solve the "every 2nd line skipped" problem


//----------memory bank file names ---------------not really needed this way, but then...//
const String memoryBank[33] ={
"mem0.txt","mem1.txt","mem2.txt","mem3.txt","mem4.txt","mem5.txt","mem6.txt", //7
"mem7.txt","mem8.txt","mem9.txt","mem10.txt","mem11.txt",//12
"mem12.txt","mem13.txt","mem14.txt","mem15.txt","mem16.txt", //17 //added another set
"mem17.txt","mem18.txt","mem19.txt","mem20.txt","mem21.txt","mem22.txt","mem23.txt",//24
"mem24.txt","mem25.txt","mem26.txt","mem27.txt","mem28.txt",//29
"mem29.txt","mem30.txt","mem31.txt","mem32.txt"//33


};
int stepBuffer[21]; //there are 20 items to recall from the file 4*step num, 4*hits, 4*offsets, 5*div values

int bothStates[16] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//array to combine bankA and bankB values

//*********rotary midi note values**************//
int Counter[16]; //this is the goto note value
int memChannelDIV[5];
int ONOFF[4];//for storing channel state when saving to mem file

//***************write to the SD card************************************//
void writeMemory(int i){

 // myFile = SD.open("memory.txt", FILE_WRITE| O_CREAT | O_TRUNC);
 myFile = SD.open(memoryBank[i].c_str(), FILE_WRITE| O_CREAT | O_TRUNC);
  // if the file opened okay, write to it:
 
  if (myFile) {
//first write the rotary step number for each rotary
for (int steps = 0; steps < 4; steps++) {
    myFile.print(euclidParams[steps][0]);
    myFile.print(",");}

//then write the hit value for each rotary into the next four places
  for (int hits = 0; hits < 4; hits++) {    
    myFile.print(euclidParams[hits][1]);
    myFile.print(",");}

//then write the offset value for each rotary into the next four places
  for (int offsets = 0; offsets < 4; offsets++) {    
    myFile.print(euclidParams[offsets][2]);
    myFile.print(",");}

//then write the DIV value for each rotary into the next four places
  for (int DIVs = 0; DIVs < 4; DIVs++) {    
    myFile.print(memChannelDIV[DIVs]);
    myFile.print(",");}

//now write the on/off state for each channel
  for (int onoff = 0; onoff < 4; onoff++) {    
    myFile.print(ONOFF[onoff]);
    myFile.print(",");}

  // close the file:
    myFile.close();
  }
  else {
    // if the file didn't open, print an error:
   Serial.println("error opening memory.txt");
  }
}


//********************read back from the SD card*****************//
//int stepBuffer[21]; //there are 20 items to recall from the file 4*step num, 4*hits, 4*offsets, 5*div values

void readMemory(int i){
  // re-open the file for reading:
    myFile = SD.open( memoryBank[i].c_str() );
  Serial.println(memoryBank[i].c_str());
    
  if (myFile) {

//load the stepbuffer with the file memory values
    for(int i=0;i<20;i++){ //set on off steps
      stepBuffer[i]=myFile.parseInt();
     }

   myFile.close();
  }
  else {
 //   Serial.println("error opening test.txt");
  }

//-----now apply the retrieved values------//
for(int x=0;x<4;x++){
  euclidParams[x][0]=stepBuffer[x];
  valRot1[x]=stepBuffer[x]; //puts the step number from read into the current
}
//--set hit values--//
for(int x=0;x<4;x++){
  euclidParams[x][1]=stepBuffer[x+4];
  valRot2[x]=stepBuffer[x+4]; //puts the beat number from read into the current
    }
//--set offset values--//
for(int x=0;x<4;x++){
  euclidParams[x][2]=stepBuffer[x+8];
  valRot3[x]=stepBuffer[x+8]; //puts the offset number from read into the current
    }
//--set DIV values--//
for(int x=0;x<4;x++){
  memChannelDIV[x]=stepBuffer[x+12];
    }
//--set onoff values--//
for(int x=0;x<4;x++){
  ONOFF[x]=stepBuffer[x+16];
    }
}
//-----------------------end of read memory---------------//
