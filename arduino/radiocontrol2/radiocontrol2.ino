

/** LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/
#include <LiquidCrystalRus.h>


#define SCREEN_WIDTH 20

#define STATUS_UNDEFINED 0
#define STATUS_GOTNAME 1
#define STATUS_GOTSTART 2
#define STATUS_GOTTITLE 3
#define STATUS_GOTREBOOT 4
#define BUFFER_SIZE 160
#define BUFFER_LAST BUFFER_SIZE-1
#define DEBOUNCE 10
#define SCREENUPDATE 200

// initialize the library with the numbers of the interface pins
LiquidCrystalRus lcd(12, 11, 5, 4, 3, 2);

#define POT_PIN  2    // select the input pin for the potentiometer

byte val = 0;       // variable to store the value coming from the sensor
byte previousValue = 255;
#define STEP 113 //1023/9 

char buffer[BUFFER_SIZE];
char strTitle[BUFFER_SIZE];
char strName[BUFFER_SIZE];
boolean bGotString;
byte bufferPTR=0;
boolean bStarted = false;
boolean bTitleUpdate = false;
boolean bNameUpdate = false;

unsigned long readTimer = 0;
unsigned long updateTimer = 0;
unsigned long currentMillis = 0;



byte string1Start, string1Stop,string2Start,string2Stop = 0;
byte scroll1Cursor,scroll2Cursor = SCREEN_WIDTH;

#define UPDATE_SCROLLER1 \
string1Start = string1Stop = 0; \
scroll1Cursor = SCREEN_WIDTH; 
#define UPDATE_SCROLLER2 \
string2Start = string2Stop = 0; \
scroll2Cursor = SCREEN_WIDTH;



void play()
{
  if (val != previousValue ) 
  {
   
       
  Serial.print("echo \"play ");
  Serial.print(val);
  Serial.print("\" | nc localhost 6600\n");
  delay(200);
  previousValue = val;
  lcd.clear();
  UPDATE_SCROLLER1
  UPDATE_SCROLLER2
  
 
  }
}
byte getValue()
{
  word rawVal = analogRead(POT_PIN);
  if (rawVal < STEP)
  {
    rawVal = STEP;
  }
  
  return rawVal/STEP-1;
  
}


void setup() {
    
 //Initialize serial and wait for port to open:
  Serial.begin(38400); 
  
  memset(strTitle,0,BUFFER_SIZE);
  memset(strName,0,BUFFER_SIZE);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  
  lcd.begin(20, 2);
  // Print a message to the LCD.
  strcpy(strName,"Radio booting...");
  
}

void loop() {
    currentMillis = millis();
    if (bGotString)
    {
      bGotString=false;
     
      if(strncmp(buffer,"AVR Start!",10)==0)
      {
        bStarted = true;
        memset(buffer,0,BUFFER_SIZE);
        bufferPTR=0;
        return;
      }
      if (bStarted)
      {
        if(strncmp(buffer,"Name: ",6)==0)
        {
          if (strncmp(strName, (buffer+sizeof(char)*6), strlen(strName)) !=0)
          {
            strcpy(strName, (buffer+sizeof(char)*6));
            strcpy(strTitle," ");
            bNameUpdate=true;
            bTitleUpdate=true;
            UPDATE_SCROLLER1
            UPDATE_SCROLLER2
            memset(buffer,0,BUFFER_SIZE);
          }
        }
      
        if(strncmp(buffer,"Title: ",7)==0)
        {
          if (strncmp(strTitle, (buffer+sizeof(char)*7), strlen(strTitle)) !=0)
          {
            strcpy(strTitle, (buffer+sizeof(char)*7));
            bTitleUpdate = true;
            UPDATE_SCROLLER2
            memset(buffer,0,BUFFER_SIZE);
          }
          
        }
      }
      
      memset(buffer,0,BUFFER_SIZE);
      bufferPTR=0;
    }
    
    
  if (bStarted)
  {
    val = getValue();    // read the value from the sensor
    if (val!=previousValue)
    {
      byte bufferValue = val;
          
      
      if(currentMillis - readTimer > DEBOUNCE) 
      {
        readTimer = currentMillis;
        val = getValue();
        if( bufferValue == val)
        {
           play();
        }
      }      
    }
  }
   
  
     if(strlen(strName) > SCREEN_WIDTH)
     {
       lcd.setCursor(scroll1Cursor, 0);
       char bfr[string1Stop-string1Start+1];
       memset(bfr,0,sizeof(bfr));
       strncpy(bfr,strName+sizeof(char)*string1Start,string1Stop-string1Start);
       lcd.print(bfr);
     }
     else
     {
      lcd.setCursor(0, 0);
      lcd.print(strName);
     }
     
     if(strlen(strTitle) > SCREEN_WIDTH)
     {
       lcd.setCursor(scroll2Cursor, 1);
       char bfr[string2Stop-string2Start+1];
       memset(bfr,0,sizeof(bfr));
       strncpy(bfr,strTitle+sizeof(char)*string2Start,string2Stop-string2Start);
       lcd.print(bfr);
     }
     else
     {
      lcd.setCursor(0, 1);
      lcd.print(strTitle);
     }
        
   
     if(currentMillis - updateTimer > SCREENUPDATE) 
     {
       updateTimer = currentMillis;
       lcd.clear();
  
       if(string1Start == 0 && scroll1Cursor > 0){
         scroll1Cursor--;
        string1Stop++;
       } else if (string1Start == string1Stop){
         string1Start = string1Stop = 0;
         scroll1Cursor = SCREEN_WIDTH;
       } else if ((string1Stop == strlen(strName)) && scroll1Cursor == 0) {
         string1Start++;
       } else {
         string1Start++;
         string1Stop++;
       }
     
       if(string2Start == 0 && scroll2Cursor > 0){
         scroll2Cursor--;
         string2Stop++;
       } else if (string2Start == string2Stop){
         string2Start = string2Stop = 0;
         scroll2Cursor = SCREEN_WIDTH;
       } else if ((string2Stop == strlen(strTitle)) && scroll2Cursor == 0) {
         string2Start++;
       } else {
         string2Start++;
         string2Stop++;
       }
     }    
  
  
}

void serialEvent(){
  while(Serial.available())
  {
    byte c = Serial.read();
    if ((c == '\r') || (c=='\n'))
    {
      bGotString = true;
      return;
    }
    if(bufferPTR < BUFFER_LAST)
    { 
         
      buffer[bufferPTR]= c;
      bufferPTR++;
      buffer[bufferPTR]=0;
    }
    else
    {
      buffer[bufferPTR]=0;
      bufferPTR=0;
      bGotString = true;
      return;
    }
    
  }
}


