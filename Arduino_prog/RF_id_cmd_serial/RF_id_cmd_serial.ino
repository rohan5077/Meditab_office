#include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 10);

char id[12];

String user_input;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  cmd_serial();
}

void all_visible(){
  int i=0;
  char local_id[12];
  if(mySerial.available()>0)
  {
    while(1)
    {
      local_id[i++] = mySerial.read();
      if(i==12)
      break;
      
    }
    Serial.print(local_id);
  Serial.println();
  }
}

void update_visible(){
  char prev_id[12];
  if(mySerial.available()>0)
  {
    for(int i=0; i<12; i++)
    {
      id[i] = mySerial.read();
      prev_id[i] = id[i];
    }
  }
    Serial.println(id);
  Serial.println();
    
  if(id == prev_id)
  Serial.print("\nSame Tag");
  
}

void invisible(){
  int j;
  if(mySerial.available()>0)
  {
    j = 1;
  }
  else
  {
    j=0;
  }
  if(j==0)
  Serial.println("ID is not detected");
  else
  Serial.println("ID is detected");
}

void cmd_serial(){

      char* user_inst[] = { "a","b","c"};
      void( *function_call[] )() = { all_visible, update_visible, invisible };
      
      if (Serial.available()){
        user_input = Serial.readStringUntil('\n');  
        delay(10);
        Serial.println(user_input);  
          
            for(int i =0; i<3; i++){
              if(user_input == user_inst[i]){
                Serial.println(i);
                function_call[i]();
                //Serial.println(p);
                //break;
              }
            }
      }
  
}

