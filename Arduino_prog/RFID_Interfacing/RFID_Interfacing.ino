
char id;

void setup()
{
  //mySerial.begin(9600); // Setting the baud rate of Software Serial Library  
  Serial.begin(9600);  //Setting the baud rate of Serial Monitor 
 }
 
void loop()
{ 
    
   while(Serial.available()>0)
    {
        id = Serial.read();
        delay(5);     
        Serial.print(id);
     }
 }
