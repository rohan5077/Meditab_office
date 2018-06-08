#include <Stepper.h>

#define STEPS 32

// create an instance of the stepper class using the steps and pins
Stepper stepper(STEPS, 8, 9, 10, 11);

int val = 0;

void setup() {
  Serial.begin(9600);
  stepper.setSpeed(200);
}

void loop() {

  if (Serial.available()>0)
  {
    val = Serial.parseInt();
    stepper.step(val);
    Serial.println(val); //for debugging
  }
 

}
