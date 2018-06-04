float metalDetected;
int monitoring;
int metalDetection = A1;
 
void setup(){
Serial.begin(9600);
}
 
void loop(){
monitoring = analogRead(metalDetection);
metalDetected = (float) monitoring*100/1024.0;
Serial.print(metalDetected);
Serial.println("%");
if (monitoring > 250)
Serial.println("Metal is Detected");
delay(1000);
 
}
