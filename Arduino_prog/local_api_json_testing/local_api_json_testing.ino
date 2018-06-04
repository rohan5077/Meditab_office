#include <YunClient.h>
#include <YunServer.h>
#include <ArduinoJson.h>
#include <HttpClient.h>


YunClient client;
HttpClient http_client;


void setup() {
  
  Serial.begin(9600);
  Bridge.begin();

  
}

void loop() {
  // not used in this example
  //char json1[] =      "{\"status\":\"success\",\"data\":true}";
  char json1[] = {    "status": "success",     "data": true };
  

            
                    StaticJsonBuffer<200> jsonBuffer;
                    JsonObject& root = jsonBuffer.parseObject(json1);
                    delay(1000);
                  
                    
                    if (!root.success()) {
                      Serial.println("parseObject() failed");
                      return;
                    }
                  
                    
                    const char* _status = root["status"];
                    bool data = root["data"]; 
                              
                    // Print values.
                    Serial.print("status : ");
                    Serial.println(_status);
                    Serial.print("data : ");
                    Serial.println(data);
            
}

