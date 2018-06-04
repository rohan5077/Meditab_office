String command;
void enable_rfid(){
   int enable_pin =11;
   Serial.println("switched on");
   pinMode(enable_pin,OUTPUT);
   digitalWrite(enable_pin,HIGH);
   command="\0\0\0\0";
     
}
void disable_rfid(){
     int enable_pin =11;
     Serial.println("switched off");
     pinMode(enable_pin,OUTPUT);
     digitalWrite(enable_pin,LOW);
     command="\0\0\0\0";
}
void read_rfid(){
  //Serial.print("reading data");
  int enable_pin =11;
  pinMode(enable_pin,OUTPUT);
  digitalWrite(enable_pin,HIGH);
  String rfid_data;
  char data_array[13];
 // unsigned char switch_off_time,data_array[13];
 //delay(200);
  if(Serial1.available()){
      rfid_data= Serial1.readStringUntil('\0');
    rfid_data.toCharArray(data_array,13);
    for(int i=0;i<13;i++){
     Serial.println(data_array[i]); 
    
    }
    
    digitalWrite(enable_pin,LOW);
  }
}
void commandproc(){
  
char* instructions[] = { "enable","disable","read"};
   void(*function_call[])( ) = { enable_rfid, disable_rfid,read_rfid };
  if(Serial.available()){
    //read_rfid();
   command = Serial.readStringUntil('\n');
  
  }
   for(int i=0;i<3;i++){
        if(command==instructions[i]){
       // Serial.
       Serial.println(instructions[i]);
        function_call[i]();
      }
    
   }
}   
void setup() {
  Serial.begin(9600);
//  Serial1.begin(9600);
  Serial.println("enter the command");
}

void loop() {
  
  void commandproc();
}
