#include <ArduinoJson.h>

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



void setup() {
 

 Serial.begin(9600);
 Bridge.begin();

 client.connect(server,10051);
 
}

void loop() {

  
  while (client.connect(server, 10051)) 
  {
        Serial.println("connected");
              
        client_status();           
        
        client.println("Connection: close" );

    // Fetch requests from server    

        http_get();
        //StaticJsonBuffer<200> jsonBuffer;        

    // Fetch JSON data from server
    
        Serial.println("JSON_DATA: ");
        if(http_client.available())
        {
          
          char json[] = {http_client.read()};
          Serial.print(json);
          
        }

  }
}

void client_status(){
  if (client.available()) {
          int c = client.read();
          Serial.print("Client value: ");
          Serial.println(c);
          delay(500);  
      }   
}

void http_get(){
  Serial.print("HTTP: ");
        Serial.println(http_client.get("http://192.168.1.157:10051/api/update_verification_device_data?rfid=1"));
}

