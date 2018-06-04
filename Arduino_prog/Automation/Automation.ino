#include <ESP8266WiFi.h>

const char* ssid ="Robot"; 
const char* password ="99$inmedi"; 
String readString;

WiFiServer server(80);

  const int analogIn = A0;
  int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
  int RawValue= 0;
  int ACSoffset = 2500; 
  double Voltage = 0;
  double Amps = 0;

void setup()
{
  Serial.begin(115200);
  server.begin();

  WiFi.begin(ssid,password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);

  pinMode(A0,INPUT);
  
}

void loop() {

    RawValue = analogRead(analogIn);
    Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
    Amps = ((Voltage - ACSoffset) / mVperAmp);
 
    Serial.print("Voltage: ");
    Serial.println(Voltage);
    Serial.print("Current: ");
    Serial.println(Amps);

  
    WiFiClient client = server.available();
      if (!client) {
        return;      
      }
      Serial.println("new client");
  
      while(!client.available()){
        delay(1);
        }

       String request = client.readStringUntil('\r');
       Serial.print("REQUEST is: ");
      Serial.println(request);
      client.flush();

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println(""); //  do not forget this one
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println("<head>");
      client.println("<title>Meditab</title>");
      client.println("</head>");
      client.println("<body>");
      client.println("<div style=\"height:60px;background-color:#a6d2c7;\">");
      client.println("<h1 style=\"padding:13px; color:#11512f; text-align: center;\">Automation</h1>");
      client.println("</div>");
      // First LIGHT
      client.println("<div style=\"height:wrap-content; background:#c6d3a7; padding:10px\">");
      client.println("<h1 style=\"margin-left:10px; color: #3b8fce\">Light:1 --></h1>");
      client.println("<input type=\"button\" value=\"ON\" onClick=window.location='?lighton1'>");
      client.println("<input type=\"button\" value=\"OFF\" onClick=window.location='?lightoff1'>");
      client.println("</div>");
      // Second LIGHT
      client.println("<div style=\"background:#c6d3a7; height:wrap-content; padding:10px\">");
      client.println("<h1 style=\"margin:10px;color: #3b8fce\">Light:2 --></h1>");
      client.println("<input type=\"button\" value=\"ON\" onClick=window.location='?lighton2'>");
      client.println("<input type=\"button\" value=\"OFF\" onClick=window.location='?lightoff2'>");
      client.println("</div>");
      client.println("</body>");
      client.println("</html>");

/*
      if(readString.indexOf("ON") >0)     // these are the snippets the Arduino is watching for.
          {
            digitalWrite(4, HIGH);
          }
          else{
          if(readString.indexOf("OFF") >0)
          {
            digitalWrite(4, LOW);
          }
  }
*/

    if (request == "GET /?lightoff1 HTTP/1.1")
      digitalWrite(4, HIGH);
    if (request == "GET /?lighton1 HTTP/1.1")
      digitalWrite(4, LOW);

     if (request == "GET /?lightoff2 HTTP/1.1")
      digitalWrite(5, HIGH);
    if (request == "GET /?lighton2 HTTP/1.1")
      digitalWrite(5, LOW);
}



  
