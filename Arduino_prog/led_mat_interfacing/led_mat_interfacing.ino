#include <MaxMatrix.h>

MaxMatrix mat(7,6,5,HIGH);

byte R[] = {7, 8, B11111111, 
                  B00010001, 
                  B00010001,
                  B00010001,
                  B00010001, 
                  B11101110, 
                  B00000000};

char A[] = {4, 8,
            B01111110,
            B00010001,
            B00010001,
            B01111110,
           };

void setup() {
  // put your setup code here, to run once:
  mat.init();
  mat.setIntensity(10);
}

void loop() {
  // put your main code here, to run repeatedly:
  mat.setDot(4, 4, HIGH);
  delay(1000);
  mat.shiftLeft(90,true);
  /*
  mat.writeSprite(2, 1, A);
  delay(1000);
  mat.writeSprite(0, 0, R);
  delay(1000);
  mat.clear();
  */
}
