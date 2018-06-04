void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0,INPUT);
  //pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int analog_R = analogRead(A0);
  //int sensor = map(analog_R,0,1023,0,100);
  //Serial.println(sensor);
    Serial.println(analog_R);
  if (analog_R>200){
    Serial.println("Object Detected");
   // digitalWrite(13,HIGH);
  //delay(1000);
  }
  else
  digitalWrite(13,LOW);
}
