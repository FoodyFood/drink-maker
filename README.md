# DrinkMaker
Arduino and ESP8266 drink dispensing and mixing machine

I will be making the ESP code shortly as I have time.

The ESP will present a web page with drink selections or perhaps simply sliders to determine what mix you want, maybe even both..

Currently the Arduino code will do the following:

//Will take the following input and pour drinks accordingly
//Takes a string, between 2 and 10 long, no escape character needed
//Will ignore anything after 10th, must be even number of bytes
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
//4: Pour ok, pending glass removal     //Will self clear when removed
//5: Glass not removed since last pour  //Will self clear when removed

This is intended to use 1 to 5 peristalic pumps on D8 -> D12 to pump the drinks.

The timer oneDrinkUnitOfTime can be configured to your taste, I will be setting it to 1/8th Unit of liquor so I can have between 0 and 2.25 units in the glass per drink. This is the time it takes you pump to move that amount of liquid (in ms).

Beware the current draw will be high as all needed pumps will activate at the same time to keep drink pouring time as short as possible and to mix the best.
