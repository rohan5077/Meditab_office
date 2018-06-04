String user_input;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  cmd_serial();
}

void a(){
  Serial.println("A is executed");
}

void b(){
  Serial.println("B is executed");
}

void c(){
  Serial.println("C is executed");
}

void cmd_serial(){

      char* user_inst[] = { "enable","disable","read"};
      void( *function_call[] )() = { a, b, c };
      
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

