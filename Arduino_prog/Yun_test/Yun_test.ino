#include <Bridge.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>

#include <SPI.h>

char server[] = "184.106.153.149";

YunClient client;

String sensor1 = "10";
int sensor2 = 65;
 String sensorvalue1 = "";
 String sensorvalue2 = "";
void setup() {
 
  // Serial.begin starts the serial connection between computer and Arduino
  Serial.begin(9600);
 Bridge.begin();
 String sensorvalue1 = "";
 String sensorvalue2 = "";
 client.connect(server,80);

 
}

void loop() {
  while (client.connect(server,80)) 
  {
    Serial.println("connected");
    
    if(Serial.available())
    sensor1 = Serial.read();

    //sensorvalue1 ="sensor1="+int(sensor1);
    //sensorvalue2 ="sensor2="+int(sensor2);

    String cmd = "GET /update?api_key=";
            cmd += "03QIZQ9DB753KMXN";
            cmd += "&field1=";
            cmd += sensor1;
    client.println(cmd);
    Serial.println(cmd);
    delay(1000);

    String ch_up  = "GET /apps/thinghttp/send_request?api_key=03QIZQ9DB753KMXN";
    Serial.println(ch_up);
    client.println(ch_up);
    delay(1000);
    

   // Serial.println(sensorvalue1.length());
   
    Serial.print("Sensor1: ");
    Serial.println(sensor1);

    
    client.println("Connection: close" );
    //client.println("Host:184.106.153.149" );//ur web server
    //client.println("Content-Type: application/x-www-form-urlencoded");
    //client.println();
   // client.println(sensorvalue1);
  //  client.get("localhost/insert.php?sensor1=20&&sensor2=21");
  }
/*    else 
    {
    Serial.println("connection failed");
    }
*/
    
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
    delay(500);  
  }
  
  
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    //for(;;)
 delay(100000);
  }
  delay(5000);
}
