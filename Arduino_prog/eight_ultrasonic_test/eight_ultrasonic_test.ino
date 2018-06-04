#define trigger 2
#define echo1 4
#define echo2 5
#define echo3 6
#define echo4 7
#define echo5 8
#define echo6 9
#define echo7 10
#define echo8 11
 
float time=0, distance=0;
 
void setup()
{
  Serial.begin(9600);
 pinMode(trigger,OUTPUT);
 pinMode(echo1,INPUT);
 pinMode(echo2,INPUT);
 pinMode(echo3,INPUT);
 pinMode(echo4,INPUT);
 pinMode(echo5,INPUT);
 pinMode(echo6,INPUT);
 pinMode(echo7,INPUT);
 pinMode(echo8,INPUT);
}
 
void loop()
{

//Ultrasonic-1

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:1");
 time=pulseIn(echo1,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
 delay(100);

//Ultrasonic-2

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:2");
 time=pulseIn(echo2,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-3

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:3");
 time=pulseIn(echo3,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-4

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:4");
 time=pulseIn(echo4,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-5

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:5");
 time=pulseIn(echo5,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-6

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:6");
 time=pulseIn(echo6,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-7

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:7");
 time=pulseIn(echo7,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
  delay(100);

//Ultrasonic-8

  digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 delayMicroseconds(2);

 Serial.println("Ultrasonic:8");
 time=pulseIn(echo8,HIGH);
 distance=time*340/20000;
 Serial.print("Distance:");
 Serial.print(distance);
 Serial.println("cm");
 Serial.print("Distance:");
 Serial.print(distance/100);
 Serial.println("m");
 Serial.println();
 
 delay(5000);
}

 
