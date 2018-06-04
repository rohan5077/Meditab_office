#include <Keypad.h>//header for keypad commands enabling

const byte ROWS = 4; // Four rows

const byte COLS = 4; // Three columns

// Define the Keymap

char keys[ROWS][COLS] = {

  {'1','2','3','A'},

  {'4','5','6','B'},

  {'7','8','9','C'},

  {'#','0','*','D'}

};

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.

byte rowPins[ROWS] = {2, 3, 4, 5 };

// Connect keypad COL0, COL1 and COL2 to these Arduino pins.

byte colPins[COLS] = {6, 7, 8, 9 };

//  Create the Keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup()

{
  Serial.begin(9600);

  for(int k=8;k<14;k++)

  {

    pinMode(k,OUTPUT);//pins 8-14 are enabled as output

  }

}

void loop()

{

  char key = kpd.getKey(); //storing pressed key value in a char

  if (key != NO_KEY)

{

      Serial.println(key); //showing pressed character on LCD

  }

}                
