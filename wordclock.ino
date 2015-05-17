
// second part


/**************************************************************************

 *                                                                         *

 *  W O R D C L O C K   - A clock that tells the time using words.         *

 *                                                                         *

 * Hardware: Arduino Dumelove with a set of individual LEDs under a word   *

 *            stencil.                                                     *

 *                                                                         *

 *   Original Copyright (C) 2009  Doug Jackson (doug@doughq.com)           *

 *   Modifications Copyright (C) 2010 Scott Bezek (scott@bezekhome.com)    *

 *                                                                         *

 ***************************************************************************

 *                                                                         * 

 * This program is free software; you can redistribute it and/or modify    *

 * it under the terms of the GNU General Public License as published by    *

 * the Free Software Foundation; either version 2 of the License, or       *

 * (at your option) any later version.                                     *

 *                                                                         *

 * This program is distributed in the hope that it will be useful,         *

 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *

 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *

 * GNU General Public License for more details.                            *

 *                                                                         *

 * You should have received a copy of the GNU General Public License       *

 * along with this program; if not, write to the Free Software             *

 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,                   *

 * MA  02111-1307  USA                                                     *

 *                                                                         *

 ***************************************************************************

 * 

 * Revision History

 * 

 * Date  	By	What

 * 20001025	DRJ	Initial Creation of Arduino Version 

 *                      - based on Wordclock.c - from PIC version

 * 20100124  Scott Bezek Changed LED pinout, added brightness control,

 *                        changed buttons to hour/minute increment 

 * 20101231  Joe Caldwell Changed pushbuttons to external pulldown,

 *                        revised comments, deleted unused code

 */


#include <Adafruit_NeoPixel.h>

#define PIN_HAPPY 2
#define PIN_BIRTH 3
#define PIN_DAY 4
#define PIN_RICK 5
#define PIN_SUSAN 6

#define HAPPYLEN 5
#define BIRTHLEN 5
#define DAYLEN 3
#define RICKLEN 4
#define SUSANLEN 5

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel stripHappy = Adafruit_NeoPixel(HAPPYLEN, PIN_HAPPY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripBirth = Adafruit_NeoPixel(BIRTHLEN, PIN_BIRTH, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripDay = Adafruit_NeoPixel(DAYLEN, PIN_DAY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripRick = Adafruit_NeoPixel(RICKLEN, PIN_RICK, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripSusan = Adafruit_NeoPixel(SUSANLEN, PIN_SUSAN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.




#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}



// Display output pin assignments

#define IT 	Display1=Display1 | (1<<0)  

#define IS	Display1=Display1 | (1<<1)

#define MTEN	Display1=Display1 | (1<<2)

#define HALF	Display1=Display1 | (1<<3)

#define QUARTER	Display1=Display1 | (1<<4)

#define TWENTY	Display1=Display1 | (1<<5)

#define MFIVE	Display1=Display1 | (1<<6)

#define MINUTES	Display1=Display1 | (1<<7)



#define PAST	Display2=Display2 | (1<<0)

#define TO	Display2=Display2 | (1<<1)

#define ONE	Display2=Display2 | (1<<2)

#define THREE	Display2=Display2 | (1<<3)

#define ELEVEN	Display2=Display2 | (1<<4)

#define FOUR	Display2=Display2 | (1<<5)

#define TWO	Display2=Display2 | (1<<6)

#define EIGHT	Display2=Display2 | (1<<7)



#define NINE	Display3=Display3 | (1<<0)

#define SEVEN	Display3=Display3 | (1<<1)

#define HFIVE	Display3=Display3 | (1<<2)

#define SIX	Display3=Display3 | (1<<3)

#define HTEN	Display3=Display3 | (1<<4)

#define TWELVE	Display3=Display3 | (1<<5)

#define OCLOCK  Display3=Display3 | (1<<6)

#define UNUSED	Display3=Display3 | (1<<7)



//int  hour=9, minute=30, second=00;

byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

static unsigned long msTick =0;  // the number of Millisecond Ticks since we last 

                                 // incremented the second counter

int  count;

char Display1=0, Display2=0, Display3=0;



// hardware constants

#define LEDClockPin 12

#define LEDDataPin 11

#define LEDLatchPin 8

#define PWMPin 9

#define MinuteButtonPin 7

#define HourButtonPin 10

//int BriteKnobPin=2; // analog read


void setup()

{
  // neopixel setup
  stripHappy.begin();
  stripHappy.show(); // Initialize all pixels to 'off'
  stripBirth.begin();
  stripBirth.show(); // Initialize all pixels to 'off'
  stripDay.begin();
  stripDay.show(); // Initialize all pixels to 'off'
  stripRick.begin();
  stripRick.show(); // Initialize all pixels to 'off'
  stripSusan.begin();
  stripSusan.show(); // Initialize all pixels to 'off'

  // initialise the hardware	
  Wire.begin();
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  //setDS3231time(30,17,17,7,16,5,15);
  
  // initialize the appropriate pins as outputs:

  pinMode(LEDClockPin, OUTPUT); 

  pinMode(LEDDataPin, OUTPUT); 

  pinMode(LEDLatchPin, OUTPUT); 

  

  //pinMode(BrightnessPin, INPUT);

  pinMode(MinuteButtonPin, INPUT); 

  pinMode(HourButtonPin, INPUT);

  

  pinMode(PWMPin, OUTPUT); 

  

  Serial.begin(19200);



  msTick=millis();      // Initialise the msTick counter

  displaytime();        // display the current time

}

/*
*  RTC Helper Functions
*/
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}





void ledsoff(void) {

 Display1=0;

 Display2=0;

 Display3=0;

 }





void WriteLEDs(void) {

 // Now we write the actual values to the hardware

 shiftOut(LEDDataPin, LEDClockPin, MSBFIRST, Display3);

 shiftOut(LEDDataPin, LEDClockPin, MSBFIRST, Display2);

 shiftOut(LEDDataPin, LEDClockPin, MSBFIRST, Display1);

 digitalWrite(LEDLatchPin,HIGH);

 delay(2);

 digitalWrite(LEDLatchPin,LOW); 

 }





void displaytime(void){



  // start by clearing the display to a known state

  ledsoff();

  

  Serial.print("It is ");
  
  IT;
  
  IS;



  // now we display the appropriate minute counter

  if ((minute>4) && (minute<10)) { 

    MFIVE; 

    MINUTES; 

    Serial.print("Five Minutes ");

  } 

  if ((minute>9) && (minute<15)) { 

    MTEN; 

    MINUTES; 

    Serial.print("Ten Minutes ");

  }

  if ((minute>14) && (minute<20)) {

    QUARTER; 

      Serial.print("Quarter ");

  }

  if ((minute>19) && (minute<25)) { 

    TWENTY; 

    MINUTES; 

    Serial.print("Twenty Minutes ");

  }

  if ((minute>24) && (minute<30)) { 

    TWENTY; 

    MFIVE; 

    MINUTES;

    Serial.print("Twenty Five Minutes ");

  }  

  if ((minute>29) && (minute<35)) {

    HALF;

    Serial.print("Half ");

  }

  if ((minute>34) && (minute<40)) { 

    TWENTY; 

    MFIVE; 

    MINUTES;

    Serial.print("Twenty Five Minutes ");

  }  

  if ((minute>39) && (minute<45)) { 

    TWENTY; 

    MINUTES; 

    Serial.print("Twenty Minutes ");

  }

  if ((minute>44) && (minute<50)) {

    QUARTER; 

    Serial.print("Quarter ");

  }

  if ((minute>49) && (minute<55)) { 

    MTEN; 

    MINUTES; 

    Serial.print("Ten Minutes ");

  } 

  if (minute>54) { 

    MFIVE; 

    MINUTES; 

    Serial.print("Five Minutes ");

  }



  if ((minute <5))

  {

    switch (hour) {

    case 1: 

      ONE; 

      Serial.println("One ");

      break;
      
    case 13: 

      ONE; 

      Serial.println("One ");

      break;

    case 2: 

      TWO; 

      Serial.println("Two ");

      break;
      
    case 14: 

      TWO; 

      Serial.println("Two ");

      break;

    case 3: 

      THREE; 

      Serial.println("Three ");

      break;
      
    case 15: 

      THREE; 

      Serial.println("Three ");

      break;

    case 4: 

      FOUR; 

      Serial.println("Four ");

      break;
      
    case 16: 

      FOUR; 

      Serial.println("Four ");

      break;

    case 5: 

      HFIVE; 

      Serial.println("Five ");

      break;
    
    case 17: 

      HFIVE; 

      Serial.println("Five ");

      break;

    case 6: 

      SIX; 

      Serial.println("Six ");

      break;
      
    case 18: 

      SIX; 

      Serial.println("Six ");

      break;

    case 7: 

      SEVEN; 

      Serial.println("Seven ");

      break;
      
    case 19: 

      SEVEN; 

      Serial.println("Seven ");

      break;

    case 8: 

      EIGHT; 

      Serial.println("Eight ");

      break;
      
    case 20: 

      EIGHT; 

      Serial.println("Eight ");

      break;

    case 9: 

      NINE; 

      Serial.println("Nine ");

      break;
      
    case 21: 

      NINE; 

      Serial.println("Nine ");

      break;

    case 10: 

      HTEN; 

      Serial.println("Ten ");

      break;
      
    case 22: 

      HTEN; 

      Serial.println("Ten ");

      break;

    case 11: 

      ELEVEN; 

      Serial.println("Eleven ");

      break;
      
    case 23: 

      ELEVEN; 

      Serial.println("Eleven ");

      break;

    case 12: 

      TWELVE; 

      Serial.println("Twelve ");

      break;
      
    case 0: 

      TWELVE; 

      Serial.println("Twelve ");

      break;

    }

  OCLOCK;

  Serial.println("O'Clock");

  }

  else

    if ((minute < 35) && (minute >4))

    {

      PAST;

      Serial.print("Past ");

      switch (hour) {

    case 1: 

      ONE; 

      Serial.println("One ");

      break;
      
    case 13: 

      ONE; 

      Serial.println("One ");

      break;

    case 2: 

      TWO; 

      Serial.println("Two ");

      break;
      
    case 14: 

      TWO; 

      Serial.println("Two ");

      break;

    case 3: 

      THREE; 

      Serial.println("Three ");

      break;
      
    case 15: 

      THREE; 

      Serial.println("Three ");

      break;

    case 4: 

      FOUR; 

      Serial.println("Four ");

      break;
      
    case 16: 

      FOUR; 

      Serial.println("Four ");

      break;

    case 5: 

      HFIVE; 

      Serial.println("Five ");

      break;
    
    case 17: 

      HFIVE; 

      Serial.println("Five ");

      break;

    case 6: 

      SIX; 

      Serial.println("Six ");

      break;
      
    case 18: 

      SIX; 

      Serial.println("Six ");

      break;

    case 7: 

      SEVEN; 

      Serial.println("Seven ");

      break;
      
    case 19: 

      SEVEN; 

      Serial.println("Seven ");

      break;

    case 8: 

      EIGHT; 

      Serial.println("Eight ");

      break;
      
    case 20: 

      EIGHT; 

      Serial.println("Eight ");

      break;

    case 9: 

      NINE; 

      Serial.println("Nine ");

      break;
      
    case 21: 

      NINE; 

      Serial.println("Nine ");

      break;

    case 10: 

      HTEN; 

      Serial.println("Ten ");

      break;
      
    case 22: 

      HTEN; 

      Serial.println("Ten ");

      break;

    case 11: 

      ELEVEN; 

      Serial.println("Eleven ");

      break;
      
    case 23: 

      ELEVEN; 

      Serial.println("Eleven ");

      break;

    case 12: 

      TWELVE; 

      Serial.println("Twelve ");

      break;
      
    case 0: 

      TWELVE; 

      Serial.println("Twelve ");

      break;

      }

    }

    else

    {

      // if we are greater than 34 minutes past the hour then display

      // the next hour, as we will be displaying a 'to' sign

      TO;

      Serial.print("To ");

      switch (hour) {

      case 1: 

        TWO; 

        Serial.println("Two ");

        break;
       
     case 13: 

        TWO; 

        Serial.println("Two ");

        break;

      case 2: 

        THREE; 

        Serial.println("Three ");

        break;
        
      case 14: 

        THREE; 

        Serial.println("Three ");

        break;

      case 3: 

        FOUR; 

        Serial.println("Four ");

        break;
        
      case 15: 

        FOUR; 

        Serial.println("Four ");

        break;

      case 4: 

        HFIVE; 

        Serial.println("Five ");

        break;
        
      case 16: 

        HFIVE; 

        Serial.println("Five ");

        break;

      case 5: 

        SIX; 

        Serial.println("Six ");

        break;
        
      case 17: 

        SIX; 

        Serial.println("Six ");

        break;

      case 6: 

        SEVEN; 

        Serial.println("Seven ");

        break;
        
      case 18: 

        SEVEN; 

        Serial.println("Seven ");

        break;

      case 7: 

        EIGHT; 

        Serial.println("Eight ");

        break;
        
      case 19: 

        EIGHT; 

        Serial.println("Eight ");

        break;

      case 8: 

        NINE; 

        Serial.println("Nine ");

        break;
        
      case 20: 

        NINE; 

        Serial.println("Nine ");

        break;

      case 9: 

        HTEN; 

        Serial.println("Ten ");

        break;
        
      case 21: 

        HTEN; 

        Serial.println("Ten ");

        break;

      case 10: 

        ELEVEN; 

        Serial.println("Eleven ");

        break;
        
      case 22: 

        ELEVEN; 

        Serial.println("Eleven ");

        break;

      case 11: 

        TWELVE; 

        Serial.println("Twelve ");

        break;
        
      case 23: 

        TWELVE; 

        Serial.println("Twelve ");

        break;

      case 12: 

        ONE; 

        Serial.println("One ");

        break;
        
      case 0: 

        ONE; 

        Serial.println("One ");

        break;

      }

    }

   WriteLEDs();

}





void incrementtime(void){

  // increment the time counters keeping care to rollover as required

  second=0;

  if (++minute >= 60) {

    minute=0;

    if (++hour == 13) {

      hour=1;  

    }

  }  

  // debug outputs

  Serial.println();

  Serial.print(hour);

  Serial.print(":");

  Serial.print(minute);

  Serial.print(":");

  Serial.println(second);

}


// rainbow for the happy birthday messages
void rainbowCycleRick(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripHappy.setPixelColor(i, WheelRick(((i * 256 / stripHappy.numPixels()) + j) & 255));
    }
    stripHappy.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripBirth.setPixelColor(i, WheelRick(((i * 256 / stripBirth.numPixels()) + j) & 255));
    }
    stripBirth.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripDay.setPixelColor(i, WheelRick(((i * 256 / stripDay.numPixels()) + j) & 255));
    }
    stripDay.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripRick.setPixelColor(i, WheelRick(((i * 256 / stripRick.numPixels()) + j) & 255));
    }
    stripRick.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t WheelRick(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return stripHappy.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripBirth.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripDay.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripRick.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return stripHappy.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripBirth.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripDay.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripRick.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return stripHappy.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripBirth.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripDay.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripRick.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void rainbowCycleSusan(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripHappy.setPixelColor(i, WheelRick(((i * 256 / stripHappy.numPixels()) + j) & 255));
    }
    stripHappy.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripBirth.setPixelColor(i, WheelRick(((i * 256 / stripBirth.numPixels()) + j) & 255));
    }
    stripBirth.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripDay.setPixelColor(i, WheelRick(((i * 256 / stripDay.numPixels()) + j) & 255));
    }
    stripDay.show();
    for(i=0; i< stripHappy.numPixels(); i++) {
      stripSusan.setPixelColor(i, WheelRick(((i * 256 / stripSusan.numPixels()) + j) & 255));
    }
    stripSusan.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t WheelSusan(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return stripHappy.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripBirth.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripDay.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   return stripSusan.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return stripHappy.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripBirth.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripDay.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   return stripSusan.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return stripHappy.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripBirth.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripDay.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
   return stripSusan.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}



void loop(void)

{ 
  
  delay(100);

  /*analogWrite(PWMPin, analogRead(0)/4); //enable dimming via potentiometer or photoresistor*/

  analogWrite(PWMPin, 0); //manually set brightness level
  
  //incrementtime();

  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  displayTime();
  
  displaytime();
  
  if(dayOfMonth == 15 & month == 8)
  {
    rainbowCycleRick(10);
  }
  if(dayOfMonth == 22 & month == 9)
  {
    rainbowCycleSusan(10);
  }

    // test to see if the Minute Button is being held down

    // for time setting

    if ( (digitalRead(MinuteButtonPin) ==1 ) && second!=1) 

      // the Minute Button is down and it has been more 

      // than one second since we last looked

    {

      minute=(((minute/5)*5) +5)%60; 

      second=0;

      second++;  // Increment the second counter to ensure that the name

      // flash doesnt happen when setting time
      
      setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
      
      displaytime();

    }



    // test to see if the Hour Button is being held down

    // for time setting

    if ((digitalRead(HourButtonPin)==1 ) && second!=1) 

    {

      minute = (minute/5)*5;  //round minute down to previous 5 min interval

      hour = (hour + 1) % 24;
      second++;  // Increment the second counter to ensure that the name

      // flash doesnt happen when setting time  
      setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

      displaytime();

    }



}
