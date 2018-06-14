char i[7];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0)
  {
    int j=0;
    while(Serial.available()>0)
    {
      i[j++] = Serial.read();
      delay(20);
    }   
     Serial.println(i);
     delay(1000);
     Serial.println("Sending from arduino");
    delay(1000);
  }
}
