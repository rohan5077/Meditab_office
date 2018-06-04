#include <Bridge.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>

#include <SPI.h>

char server[] = "192.168.1.157";

YunClient client;
HttpClient http_client;

int sensor1 = 25;

void setup() {
 

 Serial.begin(9600);
 Bridge.begin();

 client.connect(server,10051);

 
}

void loop() {
  
  while (client.connect(server, 10051)) 
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
        Serial.println(http_client.get("http://192.168.1.157:10051/api/update_verification_device_data?rfid=1"));

        Serial.print("http_client: ");
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
