#include <SPI.h>
#include <SparkFunAutoDriver.h>
#include <cmdproc.h>

#define MAX_MOTORS 2
AutoDriver board[MAX_MOTORS] = { AutoDriver(0, 10, 4),AutoDriver(1, 10, 5)}; 

HardwareSerial *_ports[]={&Serial};
unsigned char _port_status[2];
unsigned long _baud[] = { 115200 };
unsigned char _total_ports = 1;


void setup() {

    Serial.begin(9600);
  // put your setup code here, to run once:
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCK, OUTPUT);                   // SCK pin
    pinMode(SS, OUTPUT);                    // SS pin
    digitalWrite(SS, HIGH);                 // just keep it high

    SPI.begin();
    SPI.setDataMode(SPI_MODE3);
//    board[0]._numBoards= 1;  
        //stepper_init();
                         // Initial position is set to 0
      for(int i = 0;i <2; i++){
        if (board[i].getStatus()){
          Serial.print("Motor: ");
          Serial.print(i);
          Serial.println(", OK ");
        }
        else{
          Serial.print("Motor: ");
          Serial.print(i);
          Serial.println(", Not Responding ");
        }


    }

}


void loop() {
  // put your main code here, to run repeatedly:
  fwd();
  rev();
  turn90_fwd();
  turn90_rev();
}

void ping1() {
    printf("ok"); 
}

const char *_default_commands[] = { 
      "ping", // check if it's working 
      "test", // test motor
      "fw", // move forward
      "rv", // move reverse
      "tr90_f","tr90_r"
      
      "s",
      "ho",// helper tray door open command
      "hc",// helper tray door close command
      "hs"// helper tray door status 
    };

void(*_default_command_handlers[])(void) = { 
    ping1, 
    test_motor, 
    fwd, 
    rev,
    turn90_fwd,turn90_rev
    
    soft_stop,
    ht_door_open,
    ht_door_close,
    ht_door_status
    };

const unsigned char _default_total_commands = 9;

void non_blocking_function() {
    motor_task();
}

void fwd()
{
  for(int i=0; i<MAX_MOTORS; i++)
  {
     board[i].run(FWD, 200.0);
  }  
  //board[0].run(FWD, 200.0);
  //board[1].run(FWD, 200.0);
  delay(1000);
}

void rev()
{
  for(int i=0; i<MAX_MOTORS; i++)
  {
     board[i].run(REV, 200.0);
  }  
  //board[0].run(FWD, 200.0);
  //board[1].run(FWD, 200.0);
  delay(1000);
}

void turn90_rev()
{
  for(int i=0; i<MAX_MOTORS; i++)
  {
     board[i].run(REV, 90.0);
  }  
  //board[0].run(FWD, 200.0);
  //board[1].run(FWD, 200.0);
  delay(1000);
}

void turn90_fwd()
{
  for(int i=0; i<MAX_MOTORS; i++)
  {
     board[i].run(FWD, 90.0);
  }  
  //board[0].run(FWD, 200.0);
  //board[1].run(FWD, 200.0);
  delay(1000);
}
/*
void stepper_init()
{
  for ( int i=0;i<MAX_MOTORS;i++ ) {
        board[i].SPIPortConnect(&SPI);            // Before doing anything else, we need to
                                                                    //    tell the objects which SPI port to use.
                                                                    //    Some devices may have more than one.
        board[i].hardHiZ();
        board[i].setSwitchMode(SW_USER);        // Switch is not hard stop
        board[i].configSyncPin(BUSY_PIN, 0);// BUSY pin low during operations;
                                                                    //    second paramter ignored.
        board[i].configStepMode(STEP_FS_128);     // 0 microsteps per step
        board[i].setSlewRate(SR_530V_us);     // Upping the edge speed increases torque.
        board[i].setOCThreshold(OC_375mA);    // OC threshold 750mA
        board[i].setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
        board[i].setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
        board[i].setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V

//        board[i].setLoSpdOpt(lspd_option_enabled);
      
        board[i].setMaxSpeed(2048);                // 10000 steps/s max
        board[i].setFullSpeed(102400);             // microstep below 10000 steps/s
        board[i].setAcc(100);                         // accelerate at 10000 steps/s/s
        board[i].setDec(100);

        board[i].setOscMode(INT_16MHZ_OSCOUT_16MHZ);

        board[i].setAccKVAL(120);                     // We'll tinker with these later, if needed.
        board[i].setDecKVAL(120);
        board[i].setRunKVAL(25555);
        board[i].setHoldKVAL(0);                     // This controls the holding current; keep it low.
        board[i].setPos(0);      

  }

}

*/
