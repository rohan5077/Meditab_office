void setup() {
  // put your setup code here, to run once:
  pinMode(7,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(7,HIGH);
  delay(500);
  digitalWrite(7,LOW);
  digitalWrite(6,HIGH);
  delay(500);
  digitalWrite(6,LOW);
  digitalWrite(5,HIGH);
  delay(500);
  digitalWrite(5,LOW);
  digitalWrite(4,HIGH);
  delay(500);
  digitalWrite(4,LOW);
  delay(2000);
}
