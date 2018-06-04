#include <YunClient.h>
#include <YunServer.h>
#include <ArduinoJson.h>
#include <HttpClient.h>

char server[] = "192.168.1.157";

YunClient client;
HttpClient http_client;

void setup() {
  
  Serial.begin(9600);
  Bridge.begin();

 client.connect(server,10051);
  
}

void loop() {
  // not used in this example
  char json1[] =      "{\"sensor\":\"gps\",\"time\":"1351824120",\"data\":[48.756080,2.302038]}";
  char json[50];
  while (client.connect(server, 10051)) 
  {
           Serial.println("connected");
           delay(100);
                        
                  client_status(); 
                  Serial.println(client.println("Connection: close" ));

           http_get();           
           while (http_client.available()){
              char json_format[] = {http_client.read()};
              Serial.print(json_format);
              for(int i=0 ; i!= '\n'; i++)
              json[i] = json_format[i];
           }
           Serial.println();

           local_api(char json1, char* sensor, char time)

                /*
            StaticJsonBuffer<80> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(json);
            delay(1000);
          
            
            if (!root.success()) {
              Serial.println("parseObject() failed");
              return;
            }
          
            
            const char* sensor = root["sensor"];
            long time = root["time"];
            double latitude = root["data"][0];
            double longitude = root["data"][1];
          
            // Print values.
            Serial.println(sensor);
            Serial.println(time);
            Serial.println(latitude, 6);
            Serial.println(longitude, 6);
            */

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

void local_api(char js, char* key, char val)
{
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(js);
            delay(1000);
          
            
            if (!root.success()) {
              Serial.println("parseObject() failed");
              return;
            }
          
            
            const char* sensor = root[key];
            long time = root[val]; 
                      
            // Print values.
            Serial.println(sensor);
            Serial.println(time);
}


