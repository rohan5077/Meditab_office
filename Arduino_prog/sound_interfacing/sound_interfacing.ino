int ledPin=8;
int sensorPin=A0;
int val1 = 0;
int val2 = 0;

void setup(){
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin (9600);
}
  
void loop (){
  val1 =analogRead(sensorPin);
  Serial.println (val1);
  val2 =analogRead(sensorPin);
  Serial.println (val2);
  delay(1);
  // when the sensor detects a signal above the threshold value, LED flashes
  if ((val1-val2)>1) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
}

