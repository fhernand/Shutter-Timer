
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

long Start;   // this is the time in microseconds that the shutter opens (the arduino runs a microsecond clock in the background always - it is reasonably accurate for this purpose)
long Stop;    // this is the time in microseconds that the shutter closes
int Fired = 0;  // this is a flag indicating when the shutter has been fired completely.  when fired =1, the shutter has been fired, and the computer needs to display the information related to the exposure time.
int Risingflag = 0;  // this is a flag that i set in my interrupt routine, Rising flag is set to = 1 when the voltage INCREASES in the interrupt
int Fallingflag = 0;  // this is a flag that i set in the interrupt routine, Fallingflag is set to =1 when the voltage DECREASES in the interrupt



void setup() {                                                  //This part of the program is run exactly once on boot

  Serial.begin(9600);                                          //opens a serial connection.
  attachInterrupt(digitalPinToInterrupt(2), CLOCK, CHANGE);    //run the function CLOCK, every time the voltage on pin 2 changes.

  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {                                                  // this part of the program is run, in order, over and over again, start to finish, unless INTERRUPTED by our interrupt
  if(Risingflag ==1){                       
    Start = micros();       //set the variable Start to current microseconds
  Risingflag=0;           //reset the rising flag to 0, so that this function isnt called again until the shutter actually fires
  }
  if(Fallingflag == 1){
  Stop = micros();      // set the variable Stop to current microseconds
  Fallingflag = 0;      //reset the falling flag to 0, so that this function isnt called again untill the shutter fires again.
  Fired = 1;            // set the fired flag to 1, triggering the calculation of a shutter speed, and its display over the serial monitor.
  }
  if(Fired == 1){                            //if the flag Fired = 1, print this information to the serial monitor"
    Serial.print("Start: ");
    Serial.println(Start);
    Serial.print("Stop: ");
    Serial.println(Stop);
    long Speed = (Stop - Start);      // make a variable called speed, which is the total number of microseconds that the shutter is open for
    Serial.print("Microseconds: ");
    Serial.println(Speed);               //display total microseconds in shutter interval
  

    float SS = (float)Speed/1000000;    // make a variable SS, which is how many seconds that the shutter open for
    
    char shutterTime[20] = "";
    char shutterTimeInverse[18] = "";
    Serial.print("shutter speed: ");    
    if(SS >= 1){
      dtostrf(SS,0,3,shutterTime);
    } else {
      dtostrf(1/SS,0,3,shutterTimeInverse);
      sprintf(shutterTime,"1/%s",shutterTimeInverse);
    }

    Serial.println(shutterTime);                //display the shutter speed
    Serial.println();
    printShutterTime(shutterTime, ST77XX_RED);
    Start = 0;                         // reset Start to 0
    Stop = 0;                           //reset Stop to 0 . *** these are not necessarily needed, but makes errors more evident should they occur
    Fired = 0;                          //reset Fired flag to 0, so that the shutter speed will not be calclulated and displayed, until the next full interrupt cycle, where a start and stop time are generated.
  } 
}

void printShutterTime(char *text, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  tft.println("Shutter speed (s):");
  tft.setCursor(10, 30);  
  tft.setTextSize(2);
  tft.print(text);
}

void CLOCK(){                     //this is the interrupt function, which is called everytime the voltage on pin 2 changes, no matter where in the main program loop that the computer is currently in
  if(digitalRead(2) == HIGH){
    Risingflag = 1;                // if the voltage on pin 2 is high, set the Risingflag to 1 : this will trigger the function called Rising from the main loop, which will set a start time
  }
  if(digitalRead(2) == LOW){        // . if the voltage on pin 2 is low, set the Fallingflag to 1 : this will trigger the function called Falling from the main loop, which will set the stop time, and also set the Fired flag to 1.
    Fallingflag =1;
  }
}
