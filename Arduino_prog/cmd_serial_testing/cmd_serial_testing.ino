String command;

  
void setup() {
  Serial.begin(9600);
//  Serial1.begin(9600);
  Serial.println("enter the command");
}

void loop() {
  
  void cmd_serial();
}

void cmd_serial(){
  
    char* instructions[] = { "enable","disable","read"};
    void  (*function_call[])(void ) = { enable_rfid, disable_rfid, read_rfid };
    if(Serial.available()){
      //read_rfid();
     command = Serial.readStringUntil('\n');
    
    }
     for(int i=0;i<3;i++){
          if(command==instructions[i]){
         // Serial.
         Serial.println(instructions[i]);
          (function_call[i]());
        }
      
     }
} 
