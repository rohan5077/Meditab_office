#include <Servo.h>// header file for controlling servo
Servo servo_test;        //initialize a servo object for the connected servo  
                
int angle = 0;    
 
void setup() 
{ 
  servo_test.attach(9);      // attach the signal pin of servo to pin9 of arduino
} 
  
void loop() 
{ 
  for(angle = 0; angle < 180; angle += 1)    // command to move from 0 degrees to 180 degrees 
  {                                  
    servo_test.write(angle);                 //command to rotate the servo to the specified angle
    //delayMicroseconds(1);                       
  } 
 
  delay(400);
  
  for(angle = 180; angle>=1; angle-=1)     // command to move from 180 degrees to 0 degrees 
  {                                
    servo_test.write(angle);              //command to rotate the servo to the specified angle
    //delayMicroseconds(1);                       
  } 

    delay(400);
}
