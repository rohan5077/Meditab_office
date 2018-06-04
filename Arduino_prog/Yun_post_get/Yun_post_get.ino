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
HttpClient http_client;

int sensor1 = 25;

void setup() {
 

 Serial.begin(9600);
 Bridge.begin();

 client.connect(server,80);

 
}

void loop() {
  
  while (client.connect(server,80)) 
  {
        Serial.println("connected");


        Serial.print("sensor1--- ");
        Serial.println(sensor1);
    
        String cmd = "GET /update?api_key=";
                cmd += "03QIZQ9DB753KMXN";
                cmd += "&field1=";
                cmd += sensor1;
        client.println(cmd);
        Serial.println(cmd);
        delay(1000);
    
    
        
    
      if (client.available()) {
          int c = client.read();
          Serial.print("Client value1: ");
          Serial.println(c);
          delay(500);  
      }
        
        
        client.println("Connection: close" );

        Serial.print("HTTP: ");
        Serial.println(http_client.get("http://api.thingspeak.com/channels/297607/status.json"));
        //Serial.println(http_client.get("https://api.thingspeak.com/channels/297607/fields/1.json?results=10"));

        Serial.print("http_client: ");
        //if (http_client.get("https://api.thingspeak.com/channels/297607/fields/1.json?results=10")){
        while (http_client.available()){
          char http_read = http_client.read();
          
          Serial.print(http_read);
          delay(10);
        }
      
      
      
      if (!client.connected()) {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        //for(;;)
     delay(100);
      }
      
      delay(2000);
  }
}
