#include <SparkFunAutoDriver.h>
#include <cmdproc.h>
#include <SPI.h>

/* Test sketch that just gets and sets values on a L6470 AutoDriver
 *
 * This is mainly a test to make sure that getting a value after setting returns
 * the same result. This is a useful test that should be run whenever the library is changed.
 */
bool motor_module_enabled = 1;                                          // enables and disables the whole module
long counter[2],prev_counter[2];                                        // used to hold the encoder readings and previous reading of the encoder values for both the motors
int test_cycle = 0;                                                     // holds the count of the pending test cycles
int test_offset =0;                                                     // holds the offset value used for testing
int prev_l6470_status[2];                                               // holds the previous value of the l6470 motor driver chips - helps in checking the status efficietly
bool one_of_the_motor_is_busy;                                          // useful to decide when to run test cycle
bool prev_one_of_the_motor_is_busy;                                     // previous value for the variable one_of_the_motor_is_busy
// Enable or disable the low-speed optimization option. With LSPD_OPT enabled,
//  motion starts from 0 instead of MIN_SPEED and low-speed optimization keeps
//  the driving sine wave prettier than normal until MIN_SPEED is reached.
bool lspd_option_enabled = 1;
bool bln_ht_door_status = 0;
const int sl1_en_pin = 56;
const int sl2_en_pin = 57;

//AutoDriver board2(0, 53, 30);
#define MAX_MOTORS 2
AutoDriver board[MAX_MOTORS] = { AutoDriver(0,53, 30),AutoDriver(0,10, 31)}; 
//String name = "";
//unsigned long temp;
//boolean tempBool;
//byte tempByte;
//float tempFloat;
//int tempInt;
//int tempInt2;
//boolean pass = true;

void move_fwd(float mm) {
    /*
    move in fw direction for given mm
    params:
    mm - unsigned millimeter
    */
    float offset = mm;
    Serial.print("offset:");
    Serial.println(offset);
    
    counter[0] = 0;
    counter[1] = 0;
    board[1].setPos(0);
    board[0].setPos(0);
    board[1].goTo(offset);
    board[0].goTo(offset);
    
}

void move_fwd() {
    /*
    helper function for the previous function to use with mux_cmdproc 
    */
    if ( paraCount == 1 ) {
        move_fwd(10);
    } else {
        move_fwd(atof(para[1]));
    }
}

void move_rev(float mm) {
    /*
    move in bw direction for given mm
    params:
    mm - unsigned millimeter
    */
    
    float offset = mm;
    Serial.print("offset:");
    Serial.println(offset);
    
    counter[0] = 0;
    counter[1] = 0;
    board[1].setPos(offset);
    board[0].setPos(offset);
    board[1].goHome();
    board[0].goHome();
}

void move_rev() {
    /*
    helper function for the previous function to use with mux_cmdproc 
    */
    
    if ( paraCount == 1 ) {
        move_rev(10);
    } else {
        move_rev(atof(para[1]));
    }
}

void test_motor() {
    /*
    sets test parameters
    params:
    indirect, from mux_cmdproc
    */
    if ( paraCount == 1 ) {
        test_cycle      = 1000; // times
        test_offset     = 256000; //mm
    } else {
        test_cycle = atoi(para[1]);
        test_offset = atoi(para[2]);
//        test_cycle = 10;
//        test_ofmfset = 25600;
    }
    move_fwd(test_offset);
    printf("test_cycle:%d\n",test_cycle);
    printf("test_offset:%d\n",test_offset);
}

void turn(long speed, int dir=0) {
    /*
    turn in given direction for given speed
    params:
    speed - unsigned long integer - specifies numbers of steps per clock tick
    dir - 0 means ccw, 1 means cw direction
    */
    
    counter[0] = 0;
    counter[1] = 0;
    if ( dir == 0 ) {
        board[0].run(FWD,speed);
        board[1].run(FWD,speed);
    } else {
        board[0].run(REV,speed);
        board[1].run(REV,speed);
    }
}

void turn() {
    /*
    helper function for the previous function to use with mux_cmdproc 
    */
    
    if ( paraCount == 3 ) {
         turn(atol(para[1]),atoi(para[2]));
    } else {
        turn(atol(para[1]),0);
    }
}

void soft_stop(){
    /*
    Stops the motors gracefully. It helps 
    if there is any test going on then cancel it 
    */
    test_cycle = 0;
    board[0].hardStop();
    board[1].hardStop();
    printf("stopped");
    
} 

bool is_st_comm_good( int i){
        /*
        checks if the communication to given motor driver is good 
        paras: 
        i - index of the motor driver, 0 for left , 1 for right
        returns:
        1 - if the communication is good
        0 - otherwise
        */
    
        float tempFloat = board[i].getAcc();
        // be careful about rounding
        tempFloat = (tempFloat == 72.76008090920998) ? 145.52016181841995 : 72.76008090920998;
        board[i].setAcc(tempFloat);
        float v2 = board[i].getAcc();
        if (abs(tempFloat-v2) > 0.1) 
        {
            //Console.print("Expected ");
            //Console.println(tempFloat, DEC);
            //Console.print("Got ");
            //Console.println(v2, DEC);
            return false;
        }
        else
        {
            return true;
        }
    }

void test_comm() {
    // check st comm
    if ( is_st_comm_good(0) ) {
        send_event("d","Motor-0 good");
    } else {
        send_event("d","Motor-0 bad");
    }
    if ( is_st_comm_good(1) ) {
        send_event("d","Motor-1 good");
    } else {
        send_event("d","Motor-1 bad");
    }
}

void init_stepper_driver(){
    /*
    Initialize the stepper driver module with required configuration
    */

    // Note: Start by setting up the pins and the SPI peripheral.
    // The library doesn't do this for you! 
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCK, OUTPUT);                   // SCK pin
    pinMode(SS, OUTPUT);                    // SS pin
    digitalWrite(SS, HIGH);                 // just keep it high

    SPI.begin();
    SPI.setDataMode(SPI_MODE3);
   // board[0]._numBoards= 1;
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
        board[i].setOCThreshold(OC_6000mA);    // OC threshold 750mA
        board[i].setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
        board[i].setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
        board[i].setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V

        board[i].setLoSpdOpt(lspd_option_enabled);
      
        board[i].setMaxSpeed(2048);                // 10000 steps/s max
        board[i].setFullSpeed(102400);             // microstep below 10000 steps/s
        board[i].setAcc(100);                         // accelerate at 10000 steps/s/s
        board[i].setDec(100);

        // osc configurations:
        // board[0] produces clock using internal osc for 16Mhz
        // board[1] uses the osc_out from board[0]
//        if ( i == 0 ) {
//                board[0].setOscMode(INT_16MHZ_OSCOUT_16MHZ);
//        } else {
//                board[i].setOscMode(EXT_16MHZ_OSCOUT_INVERT);
//        }
        // osc configurations:
        board[i].setOscMode(INT_16MHZ_OSCOUT_16MHZ);

        board[i].setAccKVAL(120);                     // We'll tinker with these later, if needed.
        board[i].setDecKVAL(120);
        board[i].setRunKVAL(25555);
        board[i].setHoldKVAL(0);                     // This controls the holding current; keep it low.
        board[i].setPos(0);                         // Initial position is set to 0

    }
    test_comm();
}

void motor_task() {
    /*
    the non-blocking function the motor module 
    */
    one_of_the_motor_is_busy = 0;
    bool print_motor_debug_info = 1;
    // get the status and set appropriate flags
    for ( int i=0;i<MAX_MOTORS;i++) {
        int status = board[i].getStatus();
        if ( status != prev_l6470_status[i] ) {
            prev_l6470_status[i] = status;
            if (print_motor_debug_info) {
                
                Serial.print("[");
                Serial.print(i);
                Serial.print(",{\"S\":\"");
                Serial.print(status);
                Serial.print(",\"d\":\"");
            }
            // analyze the status 
            if ( (status & 0x0060) != 0 ) {
                // if the motor is running 
                one_of_the_motor_is_busy = 1;

                if ( print_motor_debug_info) {
                    Serial.print("BS,");
                }
            }     
            
            // TODO find out what the status change is and print it
            /*
            // Status register bit renames- read-only bits conferring information about the
            //  device to the user.
            #define STATUS_HIZ                     0x0001 // high when bridges are in HiZ mode
            #define STATUS_BUSY                    0x0002 // mirrors BUSY pin
            #define STATUS_SW_F                    0x0004 // low when switch open, high when closed
            #define STATUS_SW_EVN                  0x0008 // active high, set on switch falling edge,
                                                                //  cleared by reading STATUS
            #define STATUS_DIR                     0x0010 // Indicates current motor direction.
                                                                //  High is FWD, Low is REV.
            #define STATUS_NOTPERF_CMD             0x0080 // Last command not performed.
            #define STATUS_WRONG_CMD               0x0100 // Last command not valid.
            #define STATUS_UVLO                    0x0200 // Undervoltage lockout is active
            #define STATUS_TH_WRN                  0x0400 // Thermal warning
            #define STATUS_TH_SD                   0x0800 // Thermal shutdown
            #define STATUS_OCD                     0x1000 // Overcurrent detected
            #define STATUS_STEP_LOSS_A             0x2000 // Stall detected on A bridge
            #define STATUS_STEP_LOSS_B             0x4000 // Stall detected on B bridge
            #define STATUS_SCK_MOD                 0x8000 // Step clock mode is active
*/
            if ( print_motor_debug_info ) {
                if ( (status & STATUS_HIZ) == STATUS_HIZ) {
                    Serial.print("HZ,");
                }
                if ( (status & STATUS_STEP_LOSS_A) == 0) {
                    Serial.print("LA,");
                }
                if ( (status & STATUS_STEP_LOSS_B) == 0) {
                    Serial.print("LB,");
                }
                if ( (status & STATUS_OCD) == 0) {
                    Serial.print("OC,");
                }
                if ( (status & STATUS_TH_WRN) == 0) {
                    Serial.print("TW,");
                }
                if ( (status & STATUS_TH_SD) == 0) {
                    Serial.print("TS,");
                }
                if ( (status & STATUS_UVLO) == 0) {
                    Serial.print("UV");
                }
                Serial.print("\"}]\n");
            }
            
        }
    }

    // check for test cycle
    // if the motors are free then check if we have any more test_cycles pending
    if ( prev_one_of_the_motor_is_busy != one_of_the_motor_is_busy) {
        prev_one_of_the_motor_is_busy = one_of_the_motor_is_busy;
        if (one_of_the_motor_is_busy == 0) {
            //printf("both are free");
            if ( test_cycle > 0 ) {
                Serial.print("test_cycle:");
                Serial.println(test_cycle);
                if ( test_cycle%2 == 0 ) {
                    move_fwd( test_offset );
                } else {
                    move_rev( test_offset );
                }
                test_cycle--;
            }
        }
    }

}

void ping1() {
    printf("ok"); 
}

void ht_door_status(){
    if(bln_ht_door_status){
      send_event("ht","close");
    }else{
      send_event("ht","open");
    }
  }

void ht_door_open(){
  digitalWrite(sl1_en_pin,LOW);
  digitalWrite(sl2_en_pin,LOW);
  bln_ht_door_status = 0;
  ht_door_status();
  }

void ht_door_close(){
  digitalWrite(sl1_en_pin,HIGH);
  digitalWrite(sl2_en_pin,HIGH);
  bln_ht_door_status = 1;
  ht_door_status();
  }

void init_solenoid(){
  pinMode(sl1_en_pin,OUTPUT);
  pinMode(sl2_en_pin,OUTPUT);
  
  digitalWrite(sl1_en_pin,HIGH);
  digitalWrite(sl2_en_pin,HIGH);
  ht_door_close();
  }

char str_buffer[20];    // used by send_event functions

void send_event(char *k ,char *v) {
    /*
    used to send given key k and v value in a json format
    */
    sprintf(str_buffer,"{\"%s\":\"%s\"}",k,v);
    Serial.println(str_buffer);
    #ifdef ENABLE_BRIDGE
        Console.println(str_buffer);
    #endif
}

void send_event(char *k ,float v) {
    /*
    used to send given key k and v value in a json format
    */
    sprintf(str_buffer,"{\"%s\":\"",k); 
    Serial.print(str_buffer);
    Serial.print(v);
    #ifdef ENABLE_BRIDGE
        Console.print(str_buffer);
        Console.print(v);
    #endif

    Serial.println("\"}");
    #ifdef ENABLE_BRIDGE
        Console.println("\"}");
    #endif

}

HardwareSerial *_ports[]={&Serial};
unsigned char _port_status[2];
unsigned long _baud[] = { 115200 };
unsigned char _total_ports = 1;


const char *_default_commands[] = { 
      "ping", // check if it's working 
      "test", // test motor
      "fw", // move forward
      "rv", // move reverse
      "tr",
      
      "s",
      "ho",// helper tray door open command
      "hc",// helper tray door close command
      "hs"// helper tray door status 
    };

void(*_default_command_handlers[])(void) = { 
    ping1, 
    test_motor, 
    move_fwd, 
    move_rev,
    turn,
    
    soft_stop,
    ht_door_open,
    ht_door_close,
    ht_door_status
    };

const unsigned char _default_total_commands = 9;

void non_blocking_function() {
    motor_task();
}

void init_modules(){
  init_stepper_driver();
  init_solenoid();
  }

void setup_mb() {
    /*
    Initialize the motherboard
    */
    set_command_function_map(_default_commands,_default_command_handlers,_default_total_commands,non_blocking_function);
    init_modules();
    
}

void setup(){
  Serial.begin(9600); 
  setup_cmdproc((SerialPort**)_ports, _baud, _port_status, _total_ports);
  setup_mb();
}

void loop(){
  wait_for_command();
  process_command();
}
