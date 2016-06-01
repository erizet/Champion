//#include <TimerOne.h>
#include <RBD_Timer.h>
#include <RBD_Button.h>
#include "LPD6803.h"
#include <elapsedMillis.h>


//Example to control LPD6803-based RGB LED Modules in a strand
// Original code by Bliptronics.com Ben Moyes 2009
//Use this as you wish, but please give credit, or at least buy some of my LEDs!

// Code cleaned up and Object-ified by ladyada, should be a bit easier to use

/*****************************************************************************/

enum Stage
{
  Init,
  WaitToStart,
  Play,
  End,
  Restart
};


// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 2;       // 'yellow' wire
int clockPin = 3;      // 'green' wire
int buttonPin = 11;
int buttonLedPin = 12;

unsigned int lampColor;
unsigned int lampOffColor;

LPD6803 strip = LPD6803(10, dataPin, clockPin);
RBD::Button button(buttonPin); // input_pullup by default

Stage currentStage = Init;


elapsedMillis elapsedTime; 
unsigned long timeToPlay;
int winnerLamp;

void setup() {
  lampColor = Color(63,10,10);
  lampOffColor = Color(0,0,0);
  randomSeed(analogRead(0));
  Serial.begin(115200);
  Serial.println("Champion started.");
  randomSeed(analogRead(0));
  Serial.println("Random seeded.");

  pinMode(buttonLedPin, OUTPUT);           // set pin to input
  
  
  // The Arduino needs to clock out the data to the pixels
  // this happens in interrupt timer 1, we can change how often
  // to call the interrupt. setting CPUmax to 100 will take nearly all all the
  // time to do the pixel updates and a nicer/faster display, 
  // especially with strands of over 100 dots.
  // (Note that the max is 'pessimistic', its probably 10% or 20% less in reality)
  
  strip.setCPUmax(50);  // start with 50% CPU usage. up this if the strand flickers or is slow
  
  // Start up the LED counter
  strip.begin();
  Serial.println("Strip started.");

  // Update the strip, to start they are all 'off'
  strip.show();

  Serial.println("Setup complete.");
}


void loop() {
  switch(currentStage)
  {
    case Init:
      Serial.println("Stage: Init");
      currentStage = WaitToStart;
      setColorOnAllPixels(Color(0,63,0));
      strip.show();
      turnOnButtonLed(true);
      button.onReleased(); // dummy read
      break;
    case WaitToStart:
      if(button.onReleased()) {
        Serial.println("Button Released in WaitToStart");
        currentStage = Play;
        elapsedTime = 0;
        timeToPlay = random(10,60) * 1000;
        Serial.print("timeToPlay: ");
        Serial.println(timeToPlay);
        winnerLamp = random(0,4);
        Serial.print("WinnerLamp: ");
        Serial.println(winnerLamp);
        setColorOnAllPixels(lampColor);
        strip.show();
        turnOnButtonLed(false);
      }
      break;
    case Play:
      if(elapsedTime > timeToPlay)
      {
        Serial.println("Elapsed time in Play");
        setWinnerLamp(winnerLamp);
        strip.show();
        elapsedTime = 0;
        currentStage = End;
      }
      break;
    case End:
      if(elapsedTime > 15 * 1000)
      {
        Serial.println("Elapsed time in End");
        currentStage = Restart;
        turnOnButtonLed(true);
      }
      break;
    case Restart:
      if(button.onReleased()) {
        Serial.println("Button release in Restart");
        currentStage = Init;
      }
      break;
  };
}
  // Some example procedures showing how to display to the pixels
//
//for(int i= 0; i<64; i++)
//{
//if(button.onReleased()) {
//    Serial.println("Button Released");
//  }
//  
//  digitalWrite(buttonLedPin, i%2 ? HIGH : LOW);
//
//    setColorOnAllPixels(Color(i, 0, 0));
//    strip.show();
//  
// delay(1000);
//} 
//  colorWipe(Color(63, 0, 0), 50);
//  colorWipe(Color(0, 63, 0), 50);
//  colorWipe(Color(0, 0, 63), 50);
//
//  rainbow(50);
//
//  rainbowCycle(50);
//}

void turnOnButtonLed(bool on)
{
  digitalWrite(buttonLedPin, on ? LOW : HIGH);
}

void setWinnerLamp(int lampNo) {
  for (int i=0; i < strip.numPixels(); i++) {
    if(i%2)
    {
      if(i/2==lampNo)
      {
        strip.setPixelColor(i, lampOffColor);
      }
      else
      {
        strip.setPixelColor(i, lampColor);
      }
    }
  }
}

void setColorOnAllPixels(uint16_t c) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
    if(i%2)
    {
      strip.setPixelColor(i, c);
    }
  }
}


void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 96 * 3; j++) {     // 3 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 96));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 96 * 5; j++) {     // 5 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j) % 96) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint16_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}

//Input a value 0 to 127 to get a color value.
//The colours are a transition r - g -b - back to r
unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b));
}

