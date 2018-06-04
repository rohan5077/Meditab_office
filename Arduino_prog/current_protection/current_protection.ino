String readString;

const int analogIn = A0;
  int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
  int RawValue= 0;
  int ACSoffset = 2500; 
  double Voltage = 0;
  double Amps = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0,INPUT);
  pinMode(7,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    RawValue = analogRead(analogIn);
    Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
    Amps = ((Voltage - ACSoffset) / mVperAmp);
 
    //Serial.print("Voltage: ");
    //Serial.println(Voltage);
    //Serial.print("Current: ");
    Serial.println(Amps);
    //delay(1000);

    if(Amps <= 0.03 || Amps >= 0.06)
      digitalWrite(7,LOW);
     else
     digitalWrite(7,HIGH);
      delay(1000);
      
}
