/*  Arduino code to mix drinks in parallel for cocktails from serial input
    Copyright (C) 2015  D O'Connor

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

//Precomments
//TODO: Add up all amount and see if too large for glass
//TODO: Set the 1/8th shot timer value for 2.25 Max shots

//Will take the following input and pour drinks accordingly
//Takes a string, between 2 and 10 long, no escape character needed
//Will ignore anything after 10, must be even number of bytes
//Bytes are ASCII from 0 -> 5 (drink selection) and 0 -> 9 (amount)
//String is: {drink, amount, drink, amount, drink...}

//There should be a pressupre switch under the glass connected to D7
//Glass detected should result in this pin being high
//When a glass is detected, D13 will go high, this can be used to
//light the glass in its holder or simply as verification

//Will Serial print the following status:
//0: Ready (glass present, and was removed since last pour)
//1: No glass
//2: Glass removed during pour          //can't happen, debug only, will serial print 1)
//3: Pour in progress                   //Only sent during pour, will not return 3
//4: Pour ok, pending glass removal     //Will self clear
//5: Glass not removed since last pour  //Will self clear

//IO
//Pins 8-12 are Pumps
//D13 is LED
#define glassDetectInput 7

//Constants
#define oneDrinkUnitOfTime 500    //Set to the time the smallest drink unit takes to pump
#define SerialTimeOut 5           //In miliseconds
#define deBounceTime  10          //In half miliseconds (e.g. 10 => 5ms)

//Globals
byte result = 0;
bool glass_present = false;
bool glass_full = false; //Or partly full

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(SerialTimeOut);
  DDRB = DDRB | B00111111;      //Sets pumps to outputs D8 -> D12, D13 Light Output
  DDRD = DDRD & ~B10000000;     //Sets the glass detection pin to an input
  PORTD = PORTD & B01111111;    //Turn off pullup on D7
}

void loop()
{
  byte selection[10] = {48, 48, 48, 48, 48, 48, 48, 48, 48, 48}; //ascii '0'

  //When a drink is requested
  if (Serial.available())
  {
    delay(1);
    Serial.readBytes(selection, 10);  //Read UPTO the 10, for timeout see setup
    for (int i = 0; i < 10; i++)      //From character (ascii) to number
      selection[i] -= 48;
    Serial.flush();                   //Flush string terminations and any extra characters

    result = makeDrink(selection);    //Passing converted string to drink pourer routine
  }

  //Keep glass and ESP up to date with for current status
  updateGlassDetect();
  if(glass_present && !glass_full)
    result = 0; //Glass present, and was removed since last pour (ready)
  if(glass_present == false)
    result = 1; //No Glass
    
  Serial.println(result);
  delay(100);
}

byte makeDrink(byte * selection)  //Returns 0: All OK, 1: No glass, 2: Glass removed 3: Reserved
{                                 //Returns 4: Pour ok, 5: Glass still full from last time
  if(glass_full)
    return 5; //5: Glass not removed since last pour
    
  byte amount[5] = {0, 0, 0, 0, 0}; //Amount of time units each drink will pour for
  byte drinksToBePoured = 0;

  for (int i = 0; i < 10; i += 2)
  {
    if (selection[i] == 1)
    {
      drinksToBePoured |= 0x1;
      amount[0] = selection[i + 1];
    }
  }
  for (int i = 0; i < 10; i += 2)
  {
    if (selection[i] == 2)
    {
      drinksToBePoured |= 0x2;
      amount[1] = selection[i + 1];
    }
  }
  for (int i = 0; i < 10; i += 2)
  {
    if (selection[i] == 3)
    {
      drinksToBePoured |= 0x4;
      amount[2] = selection[i + 1];
    }
  }
  for (int i = 0; i < 10; i += 2)
  {
    if (selection[i] == 4)
    {
      drinksToBePoured |= 0x8;
      amount[3] = selection[i + 1];
    }
  }
  for (int i = 0; i < 10; i += 2)
  {
    if (selection[i] == 5)
    {
      drinksToBePoured |= 0x10;
      amount[4] = selection[i + 1];
    }
  }

  //Get maximum pour time
  byte longestPour = 0;
  for (int i = 0; i < 5; i++)
    if (amount[i] > longestPour)
      longestPour = amount[i];

  //Check for a glass, if present start the pumps
  updateGlassDetect();
  if (glass_present == true)
  {
    PORTB = PORTB | drinksToBePoured; //Turns on the pumps selected above
    glass_full = true;
  }
  else return 1;  //1: Glass was never present

  byte drinkMeasuresPouredAlready = 0;
  while (drinkMeasuresPouredAlready <= longestPour)
  {
    if (amount[0] <= drinkMeasuresPouredAlready) //If this drink has poured for long enough
      PORTB = PORTB & ~B00000001;   //Leaves on all but the first (if already on)

    if (amount[1] <= drinkMeasuresPouredAlready)
      PORTB = PORTB & ~B00000010;   //Leaves on all but the second (if already on)

    if (amount[2] <= drinkMeasuresPouredAlready)
      PORTB = PORTB & ~B00000100;

    if (amount[3] <= drinkMeasuresPouredAlready)
      PORTB = PORTB & ~B00001000;

    if (amount[4] <= drinkMeasuresPouredAlready)
      PORTB = PORTB & ~B00010000;

    Serial.println(3); //Pour in progress

    delay(oneDrinkUnitOfTime);  //Let pumps pour for 1 unit of time
    drinkMeasuresPouredAlready++;

    updateGlassDetect();  //See if user removed glass
    if (!glass_present)
      return 2; //2: Glass removed during pour

    Serial.print("Pouring: ");
    Serial.println(drinkMeasuresPouredAlready);
  }
  return 4; //4: Pour successful, pending glass removal
}

void updateGlassDetect()
{
  byte pressedCount = 0;
  for (int i = 0; i < deBounceTime; i++)
  {
    if (digitalRead(glassDetectInput) == true)
      pressedCount++;
    delayMicroseconds(500);
  }
  if (pressedCount == deBounceTime)
  {
    glass_present = true;
    PORTB |= B00100000;   //Turn on light D13
  }
  else
  {
    glass_present = false;
    PORTB &= B11000000;   //Turn off light + pumps
    glass_full = false;
  }
}
