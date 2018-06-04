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

int sensor1 = 64789;

void setup() {
 
  // Serial.begin starts the serial connection between computer and Arduino
  Serial.begin(9600);
 Bridge.begin();

 client.connect(server,80);

 
}

void loop() {
  if (client.connect(server,80)) 
  {
    Serial.println("connected");
  
    //client.print("POST /syncnow?rfid=");
    delay(500);

    Serial.println(sensor1);

    client.print(sensor1);
    //client.print("HTTP/1.1");
    
   client.println("Connection: close" );
      //client.println("Host: 192.168.1.139" );//ur web server

    client.println();
    //client.println(sensorvalue1);
  //  client.get("localhost/insert.php?sensor1=20&&sensor2=21");
  }
    else 
    {
    Serial.println("connection failed");
    }
    
  if (client.available()) {
    char c = client.read();
    Serial.print(c);  
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    for(;;)
 ;
 delay(10000);
  }
}
