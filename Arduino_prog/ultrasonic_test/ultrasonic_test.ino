#define trigger 8
#define echo 9
 
float time=0, distance=0;
 
void setup()
{
  Serial.begin(9600);
 pinMode(trigger,OUTPUT);
 pinMode(echo,INPUT);
}
 
void loop()
{
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 time=pulseIn(echo,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 delay(1000);
}

 
