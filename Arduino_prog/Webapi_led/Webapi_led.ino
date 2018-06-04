#include <Bridge.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>

IPAddress server(192,168,1,200); 
YunClient client;

String parametri="";
int x1,x2,x3;

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  String parametri ="";
  delay(2500);
  Serial.println("connecting...");
  x1 = analogRead(A0);
  x2 = analogRead(A1);
  x3 = analogRead(A2);
  if (client.connect(server, 80)) {
    Serial.println("connected");
    delay(2500);
    parametri="x1="+String(x1)+"&x2="+String(x2)+"&x3="+String(x3);
    client.println("POST /arduino/index.php HTTP/1.1");
    client.print("Content-length:");
    client.println(parametri.length());
    Serial.println(parametri.length());
    Serial.println(parametri);
    client.println("Connection: Close");
    client.println("Host:192.168.1.200");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(parametri);     
 } else {
   Serial.println("connection failed");
 }
}

void loop() {
  if (client.available()) {
    char c = client.read();
    Serial.print(c);  
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    for(;;);
  }
}
