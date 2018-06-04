#include "Arduino.h"
#include <robot6_stepper_driver.h>
#include <avr/wdt.h>

#ifdef USE_MODEX_STACK
    #include <SparkFunAutoDriver.h>
#else
    #include <robot6_wire.h>
#endif

#define BIT(b) (1<<(b))

/* variables */
//const int MAX_MOTORS = 8; // this is maximum supported motors
float step_interval[MAX_MOTORS]; // // sudo unit
float acc_step[MAX_MOTORS]; // // sudo unit
float dec_step[MAX_MOTORS]; // // sudo unit
float power_kval[MAX_MOTORS];  // sudo unit

unsigned int current[MAX_MOTORS]; // milli amp
unsigned int pulse_count[MAX_MOTORS];
bool b_motor_busy[MAX_MOTORS];
bool b_st_comm_good[MAX_MOTORS];
bool motor_state[MAX_MOTORS]; // 0 - off, 1 - on
unsigned int current_limit_pin[MAX_MOTORS]; // used by timer interrupt to check on limit pin


unsigned int ramp_steps[MAX_MOTORS]; // step count
unsigned int max_possible_step_interval[MAX_MOTORS]; // sudo unit 
float initial_possible_step_interval_offset[MAX_MOTORS]; // sudo unit

//bool auto_power_manage[MAX_MOTORS]; // 0 - auto power manage off, 1 - auto power manage on
float timer_counter[MAX_MOTORS];  // used as a local timer per stepper motor;

/* 
Dependent motor feature
*/
bool b_monitor_dependent_motor[MAX_MOTORS+3];   // 1 - monitor dependentancy, 0 - dont 
unsigned char dependent_motor_index[MAX_MOTORS+3]; // motor index of the dependent axis
bool dependent_motor_assert_direction[MAX_MOTORS+3];  // cl - 0 ccl - 1 and for atleast one hit - 2

bool b_inside_timer = 0;
// state based accellaration feature.
// accell state constants
#define STATE_ACCEL     1
#define STATE_STABLE    2
#define STATE_DEACCEL   3

unsigned char current_state[MAX_MOTORS];

// stepper driver L297 step modes 
#define FULL_STEP   1
#define HALF_STEP   0


unsigned int pulse_count_accel[MAX_MOTORS];
unsigned int pulse_count_stable[MAX_MOTORS];
unsigned int pulse_count_deaccel[MAX_MOTORS];

float delay_step[MAX_MOTORS];
float step_interval_changer[MAX_MOTORS];
float current_step_interval[MAX_MOTORS];

int prev_l6470_status[MAX_MOTORS];
// A note about stepper pulse 
// if h_f_pin is set to 1 then its 400 pulse per revolution
// if h_f_pin is set to 0 then its 200 pulse per revolution



/////////////////////////////////////////////////////////////////////////
// Stepper Driver Related
/////////////////////////////////////////////////////////////////////////




#ifdef USE_MODEX_STACK
    long motor_position[MAX_MOTORS];    // used to store current position of the motor 
    /*
    AutoDriver board[] = {
        AutoDriver(0, A2, 4), 
        AutoDriver(1, A2, 4), 
        AutoDriver(0, 2, 4), 
        AutoDriver(1, 2, 4), 
        AutoDriver(0, 10, 4), 
        AutoDriver(1, 10, 4), 
        AutoDriver(0, 5, 4), 
        AutoDriver(1, 5, 4) 
    */
    #ifdef BASE_PLATE_V4
        /*
        AutoDriver board[] = {
            AutoDriver(0, slave_cs_0_pin, spi_reset_0_pin), 
            AutoDriver(1, slave_cs_0_pin, spi_reset_0_pin), 
            AutoDriver(0, slave_cs_1_pin, spi_reset_1_pin), 
            AutoDriver(1, slave_cs_1_pin, spi_reset_1_pin), 
            AutoDriver(0, slave_cs_2_pin, spi_reset_2_pin), 
            AutoDriver(1, slave_cs_2_pin, spi_reset_2_pin), 
            AutoDriver(0, slave_cs_3_pin, spi_reset_3_pin), 
            AutoDriver(1, slave_cs_3_pin, spi_reset_3_pin),
            AutoDriver(0, slave_cs_4_pin, spi_reset_4_pin), 
            AutoDriver(1, slave_cs_4_pin, spi_reset_4_pin) 
            
        };
        */
        
        AutoDriver board[] = {
            AutoDriver(0, 22, 32), 
            AutoDriver(1, 22, 32), 
            AutoDriver(0, 23, 33), 
            AutoDriver(1, 23, 33), 
            AutoDriver(0, 24, 34), 
            AutoDriver(1, 24, 34), 
            AutoDriver(0, 25, 35), 
            AutoDriver(1, 25, 35),
            AutoDriver(0, 26, 36), 
            AutoDriver(1, 26, 36) 
            
        };
        
    #else
        AutoDriver board[] = {
            AutoDriver(0, _xt_cs_pin, _spi_reset_pin), 
            AutoDriver(1, _xt_cs_pin, _spi_reset_pin), 
            AutoDriver(0, _xb_cs_pin, _spi_reset_pin), 
            AutoDriver(1, _xb_cs_pin, _spi_reset_pin), 
            AutoDriver(0, _pft_cs_pin, _spi_reset_pin), 
            AutoDriver(1, _pft_cs_pin, _spi_reset_pin), 
            AutoDriver(0, _zl_cs_pin, _spi_reset_pin), 
            AutoDriver(1, _zl_cs_pin, _spi_reset_pin) 
          
        };
    #endif 
    bool b_cw_limit_status[MAX_MOTORS];
    bool b_ccw_limit_status[MAX_MOTORS];
    bool b_current_direction[MAX_MOTORS];
    bool b_print_l6470_status;

    void setup_stepper() {

        //printf("[0,{\"d\":\"checking all st_comm\"}\n");
        
        // enable the debug info by default
        b_print_l6470_status = 1;

        // change max_boards to 2 because every 2 chip is grouped by a unique cs pin
        board[0]._numBoards= 2;
        
        #ifndef BASE_PLATE_V4
            // let loose all the unwanted pins 
            pinMode(11, INPUT);
            pinMode(12, INPUT);
            pinMode(13, INPUT);
        #endif
        // Start by setting up the pins and the SPI peripheral.
        //  The library doesn't do this for you! 
        pinMode(MOSI, OUTPUT);
        pinMode(MISO, INPUT);
        pinMode(52, OUTPUT);        // SCK pin
        pinMode(53, OUTPUT);        // SS pin
        digitalWrite(53, HIGH);     // just keep it high


        SPI.begin();
        SPI.setDataMode(SPI_MODE3);

        
        
        for ( int i=0;i<MAX_MOTORS;i++) {
            board[i]._numBoards= 2;    
            board[i].initPins();
        }
        
        // motor dependent feature 
        // initially all the motor dependent features are off
        for ( int i=0;i<MAX_MOTORS;i++) {
            b_monitor_dependent_motor[i] = 0;
        }
        
        // Initial configuration
        //
        // Tell the Autodriver object which SPI port you want to use. Some
        // devices may have more than one.

        for ( int i=0;i<MAX_MOTORS;i++) {
            // change max_boards to 2 because every 2 chip is grouped by a unique cs pin
            
            board[i]._numBoards= 2;
            
            board[i].SPIPortConnect(&SPI);
            board[i].hardHiZ();
            board[i].setSwitchMode(SW_USER);    // Switch is not hard stop
            board[i].describeStatus(i,board[i].getStatus(),"configSyncPin");
            
            board[i].configSyncPin(BUSY_PIN, 0);// BUSY pin low during operations;
                                          //  second paramter ignored.

            
            board[i].configStepMode(STEP_FS_128);   // 0 microsteps per step
            board[i].setAcc(acc_step[i]);             // accelerate at 10000 steps/s/s
            board[i].setDec(dec_step[i]);
            board[i].setSlewRate(SR_530V_us);   // Upping the edge speed increases torque.
            board[i].setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
            board[i].setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
            board[i].setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V
            //board[i].setSwitchMode(SW_HARD_STOP);    // Switch is not hard stop
            //board[i].describeStatus(i,board[i].getStatus(),"setSwitchMode");
            // step_interval is used for speed.
            // please note that when using L6470 the value of the step interval specifies 
            // steps per seconds (max) 
            //printf("step interval[%d]:",i);
            //Serial.println(step_interval[i]);
            
            board[i].setMaxSpeed(step_interval[i]);        // 10000 steps/s max


            board[i].setFullSpeed(2000000);       // microstep below 900000 steps/s

            
            // current specifies the threshold current value
            // please note that when using L6470 this current has to be one of the follwing
            /*          
            // setOCThreshold() options
            #define OC_375mA  0x00
            #define OC_750mA  0x01
            #define OC_1125mA 0x02
            #define OC_1500mA 0x03
            #define OC_1875mA 0x04
            #define OC_2250mA 0x05
            #define OC_2625mA 0x06
            #define OC_3000mA 0x07
            #define OC_3375mA 0x08
            #define OC_3750mA 0x09
            #define OC_4125mA 0x0A
            #define OC_4500mA 0x0B
            #define OC_4875mA 0x0C
            #define OC_5250mA 0x0D
            #define OC_5625mA 0x0E
            #define OC_6000mA 0x0F
            */
            // other than these values will result in unexpected operation
            board[i].setOCThreshold(current[i]);  // OC threshold 750mA
            //printf("current[%d]:",i);
            //Serial.println(current[i]);
            
            board[i].setOscMode(INT_16MHZ_OSCOUT_16MHZ);    // // osc configurations: internal 16 Mhz
            board[i].setAccKVAL(power_kval[i]);           // We'll tinker with these later, if needed.
            board[i].setDecKVAL(power_kval[i]);
            board[i].setRunKVAL(power_kval[i]);
            //board[i].describeStatus(i,board[i].getStatus(),"setRunKVAL");
            // holding_current sets the holdKVal
            // it doesnt represent holding current directly 
            // please check the datasheet for more information
            board[i].setHoldKVAL(holding_current[i]);           // This controls the holding current; keep it low.
      
            motor_position[i] = 0; // initially motor position is set to zero

            // initially limit switch status is open
            // it will be set by stepper_task
            b_cw_limit_status[i] = 0;
            b_ccw_limit_status[i] = 0;
            
            board[i].resetPos();
            prev_l6470_status[i] =  0xFFFF; //board[i].getStatus();
            
            //Serial.println("cw_limit_pin[i]:");
            //Serial.println(cw_limit_pin[i]);
            //Serial.println("ccw_limit_pin[i]:");
            //Serial.println(ccw_limit_pin[i]);

            // check the comm
            //printf("[%d,{\"d\":\"checking st_comm-%d\"}\n",i);
            
            if( is_st_comm_good(i) == 1) {
                printf("[%d,{\"st_comm\":\"good\"}]\n",i);
            } else {
                printf("[%d,{\"st_comm\":\"bad\"}]\n",i);
            }
            
            
        }

    }
    void set_holding_current(int i,int c) {
        holding_current[i] = c;
        board[i].setHoldKVAL(c); 
    }
    void set_accel_speed(int i,float c) {
        acc_step[i] = c;
        board[i].setAcc(c); 
    }
    void set_deaccel_speed(int i,float c) {
        dec_step[i] = c;
        board[i].setDec(c); 
    }
    void set_stepper_power(int i,float c) {
        power_kval[i] = c;
        board[i].setAccKVAL(c); 
        board[i].setDecKVAL(c); 
        board[i].setRunKVAL(c); 
    }

    void enable_stepper_debug_info(){
        b_print_l6470_status = 1;
        printf("[0,{\"d\":\"stepper debug info enabled\"}]");
    }
    void disable_stepper_debug_info() {
        b_print_l6470_status = 0;
        printf("[0,{\"d\":\"stepper debug info disabled\"}]");
    }
    void stepper_task(){
        for( int i=0;i<MAX_MOTORS;i++) {
            //Serial.print("checking ");
            //Serial.println(i);
            // get the l6470_status and set appropriate flags
            //delay(1000);
            int l6470_status = board[i].getStatus();
            
            if  ( prev_l6470_status[i] != l6470_status) {
                prev_l6470_status[i] = l6470_status;
                // check if the comm is good 
                if ( (l6470_status == 0xFFFF) || (l6470_status == 0x0000) ) {
                    if ( b_st_comm_good[i] == 1) {
                        b_st_comm_good[i] = 0;
                        printf("[%d,{\"st_comm\":\"bad\"}]\n",i);
                    }
                } else {
                    if ( b_st_comm_good[i] == 0 ) {
                        b_st_comm_good[i] = 1;
                        printf("[%d,{\"st_comm\":\"good\"}]\n",i);
                    } 
                }
                
                if ( b_print_l6470_status ) {
                    board[i].describeStatus(i,l6470_status);
                    
                }
            }
            
            if ( (l6470_status & STATUS_BUSY) == 0 ) {
                // set the b_motor_busy to 1
                b_motor_busy[i] = 1;
                // if the board is busy print the l6470_status register value
                //printf("%d,%x]\n",i,l6470_status);

                motor_position[i] = board[i].getPos();
            } else {
                b_motor_busy[i] = 0;
            }
            // updating the limit switch l6470_status
            // in modex stack mode there are two modes to know about the limit switch
            // mode 1. using the hw limit switch which is similar to reading the pin l6470_status using digitalRead, 
            // in which case the pin number will be more than 0
            // mode 2. using getStatus function of the L6470, in this we assume that the l6470_status of the sw pin is off the ccw limit switch
            // in both cases we update the b_cw_limi_status and b_ccw_limit_status variables with the l6470_status of the limit switch
            // mode 3. where there is no limit switch is connected, the pin number for this mode will be -1
            if ( cw_limit_pin[i] == -1) {
                b_cw_limit_status[i] = 0;
            }
            else if ( cw_limit_pin[i] == 0 ) {
                //Serial.print("special check cw read of ");
                //Serial.println(i);
                
                // mode 2 using getStatus()

            
                if ( (l6470_status & STATUS_SW_F) == STATUS_SW_F) { // low when switch open, high when closed
                    
                    b_cw_limit_status[i] = 1;
                    //board[i].resetPos();
                    //printf("cw:%d,l6470_status:%x,high\n",i,l6470_status);

                } else {
                    //Serial.print("cw not hit of ");
                    //Serial.println(i);
            
                    b_cw_limit_status[i] = 0;
                    //printf("cw:%d,l6470_status:%x,low\n",i,l6470_status);
                }
                
            } else {
                //Serial.print("normal cw read of ");
                //Serial.println(i);
                
                b_cw_limit_status[i] = !digitalRead(cw_limit_pin[i]);
                if ( b_motor_busy[i] == 1) {
                    if ( (b_current_direction[i] == DIRECTION_CW ) && (b_cw_limit_status[i] == 1) ){
                        // if the motor is running cw direction then stop it immediatly
                        board[i].hardStop();
                        //printf("motor[%d] hardStopped(cw)\n",i);
                    }
                }
            }

            if ( ccw_limit_pin[i] == -1) {
                b_ccw_limit_status[i] = 0;
            }
            else if ( ccw_limit_pin[i] == 0 ) {
                //Serial.print("special check ccw read of ");
                //Serial.println(i);
                
                // mode 2 using getStatus()

                if ( (l6470_status & STATUS_SW_F) == STATUS_SW_F) { // low when switch open, high when closed
                    //Serial.print("ccw hit of ");
                    //Serial.println(i);
                    
                    b_ccw_limit_status[i] = 1;
                    //board[i].resetPos();
                    //printf("ccw:%d,l6470_status:%x,high\n",i,l6470_status);
                } else {
                    b_ccw_limit_status[i] = 0;
                    //printf("ccw:%d,l6470_status:%x,low\n",i,l6470_status);
                }
            
            } else {
                //Serial.print("normal ccw read of ");
                //Serial.println(i);
                        
                b_ccw_limit_status[i] = !digitalRead(ccw_limit_pin[i]);
                if ( b_motor_busy[i] == 1) {
                    if ( (b_current_direction[i] == DIRECTION_CCW ) && (b_ccw_limit_status[i] == 1) ){
                        // if the motor is running cw direction then stop it immediatly
                        board[i].hardStop();
                        //printf("motor[%d] hardStopped(ccw)\n",i);
                    }
                }
            }

            // checking on motor dependent feature 
            // check for dependent feature 
            if ( b_motor_busy[i] == 1) {
                if ( b_monitor_dependent_motor[i] == 1) {
                    if ( dependent_motor_assert_direction[i] == 0) {
                        // 0 is for cw limit switch not hti
                        if ( b_cw_limit_status[dependent_motor_index[i]] == 0) {
                            // send the event and 
                            board[i].hardStop();
                           
                        }

                    } else if ( dependent_motor_assert_direction[i] == 1) {
                        // 1 is for ccw limit switch not hit 
                        if ( b_ccw_limit_status[dependent_motor_index[i]] == 0) {
                            // stop the motor 
                            board[i].hardStop();
                        }
                    }  else if ( dependent_motor_assert_direction[i] == 2) {
                        // 2 is for atleast one of the limit switch should be hit  
                        if ((b_cw_limit_status[dependent_motor_index[i]] == 0) && 
                            (b_ccw_limit_status[dependent_motor_index[i]] == 0)) {
                            // stop the motor 
                            board[i].hardStop();
                        }
                    }
                }
            }
        }
    }
    bool is_st_comm_good( int i){
        // currently max speed is set to some known value which is 900000
        // so we read it after setting it and check if it returns the same 
        
        float tempFloat;
        float prevValue;
        float v2;

        prevValue = board[i].getAcc();
        // be careful about rounding
        tempFloat = (tempFloat == 72.76008090920998) ? 145.52016181841995 : 72.76008090920998;
        // try for 10 times 
        for ( int x=0;x<10;x++) {
            board[i].setAcc(tempFloat);
            v2 = board[i].getAcc();
            if (abs(tempFloat-v2) <= 0.1) 
            {
                board[i].setAcc(prevValue); // restore the previous value
                return (1);
            } else {
                /*Serial.print("Expected ");
                Serial.println(tempFloat, DEC);
                Serial.print("Got ");
                Serial.println(v2, DEC);
                */
            }
        }

        board[i].setAcc(prevValue); // restore the previous value

        return 0;
    }
    
    
    void get_sw_mode(int i) {
        Serial.print("board[");
        Serial.print(i);
        Serial.print("].getSwitchMode()=");
        if ( board[i].getSwitchMode() == SW_USER ) {
            Serial.println("SW_USER");
        } else {
            Serial.println("HARD_MODE");
        }
        
      
    }

    void set_sw_mode(int i, unsigned int mode) {
        board[i].setSwitchMode(mode);
        //board[i].softStop();    // fix for not communicating properly
        get_sw_mode(i);
    }

    
    void enable_stepper_timer(){}
    void disable_stepper_timer(){}
    void configure_stepper_limit_switches(int i, int new_cw_limit_pin, int new_ccw_limit_pin){}
    void cw_limit_status(int i) {
        printf("{\"cw-%d\":\"",i);
        if ( b_cw_limit_status[i] ) {
            printf("hit");
        } else {
            printf("not hit");
        }
        printf("\"}");
    }

    void ccw_limit_status(int i) {
        printf("{\"ccw-%d\":\"",i);
        if ( b_ccw_limit_status[i] ) {
            printf("hit");
        } else {
            printf("not hit");
        }
        printf("\"}");
    }
    bool is_cw_hit(int i){
        return ( b_cw_limit_status[i] );
    }
    bool is_ccw_hit(int i){
        return ( b_ccw_limit_status[i] );

    }
    void set_step_interval(int i,float v){
        // aka set speed
        // step_interval is used for speed.
        // please note that when using L6470 the value of the step interval specifies 
        // steps per seconds (max) 
        step_interval[i] = v;
        board[i].setMaxSpeed(step_interval[i]);        // 10000 steps/s max
        
    }
    void set_current(int i,unsigned int v){
        // current specifies the threshold current value
        // please note that when using L6470 this current has to be one of the follwing
        /*          
        // setOCThreshold() options
        #define OC_375mA  0x00
        #define OC_750mA  0x01
        #define OC_1125mA 0x02
        #define OC_1500mA 0x03
        #define OC_1875mA 0x04
        #define OC_2250mA 0x05
        #define OC_2625mA 0x06
        #define OC_3000mA 0x07
        #define OC_3375mA 0x08
        #define OC_3750mA 0x09
        #define OC_4125mA 0x0A
        #define OC_4500mA 0x0B
        #define OC_4875mA 0x0C
        #define OC_5250mA 0x0D
        #define OC_5625mA 0x0E
        #define OC_6000mA 0x0F
        */
        // other than these values will result in unexpected operation
        current[i] = v;
        board[i].setOCThreshold(current[i]);  // OC threshold 750mA
    }
    void set_ramp_steps(int i,unsigned int v){}
    void get_step_interval(int i) {
        //velocity - inches per second
        printf("{\"step_interval-%d\":",i);
        Serial.print(board[i].getMaxSpeed());
        Serial.println("}");


    }
    void get_current(int i) {
        printf("{\"current-%d\":",i);
        Serial.print(board[i].getOCThreshold());
        Serial.println("}");
    }
    void get_ramp_steps(int i) {
        printf("{\"ramp_steps-%d\":\"%d\"}",i,ramp_steps[i]);
    }
    void set_voltage(int i,int v){}
    void motor_on(int i){}
    void motor_on(int i,int v){}
    void motor_off(int i){
        board[i].hardStop();
    }
    void move_cw(int i,long v){
        Serial.print("[");
        Serial.print(i); 
        Serial.print(",(\"move_cw\",\"");
        Serial.print(v);
        Serial.println("\")]");
        

        // check the limit switch
        if ( cw_limit_pin[i] > 0) {
            if ( digitalRead(cw_limit_pin[i]) == 0) {
                // limit switch is hit 
                // do not move
                //printf("{\"d-%d\":\"limit_hit\"}",i);
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"b\",");
                Serial.print(i);
                Serial.println(")]");
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"m\",");
                Serial.print(i);
                Serial.println(")]");
                
                return;
            } else {
                // need to inform the software that the motor is busy
                // read by robot6_motor_driver
                b_motor_busy[i] = 1;
            }
        } else {
            // this is a sw switch or logical switch
            if ( b_cw_limit_status[i] == 1 ) {
                //printf("{\"d-%d\":\"limit_hit\"}",i);
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"b\",");
                Serial.print(i);
                Serial.println(")]");
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"m\",");
                Serial.print(i);
                Serial.println(")]");
                return;
            } else {
                // need to inform the software that the motor is busy
                // read by robot6_motor_driver
                
                b_motor_busy[i] = 1;
            }
        }
        
        b_current_direction[i] =  DIRECTION_CW;
        
        //#ifndef BASE_PLATE_V4
            // only for z axis perform the following 
            //if ( (i == 6) || (i == 7)) {
                // set the switchmode to user and 
                // make sure its set 

                // first make sure the sw pin is not hit 
                // if it is hit then call release 
                /*
                if ( (board[i].getStatus() & STATUS_SW_F) == STATUS_SW_F) {
                    //Serial.println("sw hit");

                    board[i].run(1,step_interval[i]);
                    while ( (board[i].getStatus() & STATUS_SW_F) == STATUS_SW_F) {
                        //Serial.println("releasing");
                    }
                    //Serial.println("wait complete");
                }
                board[i].hardStop();
                
                while ( (board[i].getStatus() & STATUS_BUSY) == 0 ) {
                        //Serial.println("still busy");
                }
                */
                int tryCount;
                tryCount = MAX_TRY_COUNT_ST_COMM;
                
                while ( board[i].getSwitchMode() != SW_USER ) {
                    board[i].hardHiZ();
                    board[i].setSwitchMode(SW_USER); // since its a cw direction we dont wanna stop even if the sw (ccl limit switch) is hit
                                                           // sometimes due to noice it can trigger falling edge event, so this helps
                    board[i].describeStatus(i,board[i].getStatus());
                    tryCount--;
                    if ( tryCount == 0 ) {
                        break;
                    }
                    /*
                    Serial.print("board[i].getSwitchMode():");
                    Serial.println(board[i].getSwitchMode());
                    Serial.print("tryCount:");
                    Serial.println(tryCount);
                    */
                }
                if ( tryCount == 0) {
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"e\",");
                    Serial.print("\"comm_error\"");
                    Serial.println(")]");
                    /*Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"b\",");
                    Serial.print(i);
                    Serial.println(")]");
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"m\",");
                    Serial.print(i);
                    Serial.println(")]");
                    */
                    //return;
                } else {
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"d\",");
                    Serial.print("\"user_mode\"");
                    Serial.println(")]");
                    
                }
            //}
        //#endif
        board[i].setPos(0);
        board[i].goTo(v);
        //printf("{\"cw-v-%d\":\"",i);
        //Serial.print(v);
        //printf("\"}");
    }
    void move_ccw(int i,long v){
        Serial.print("[");
        Serial.print(i); 
        Serial.print(",(\"move_ccw\",\"");
        Serial.print(v);
        Serial.println("\")]");
        
        // check the limit switch
        if ( ccw_limit_pin[i] > 0) {
            // there is a hard limit switch so check if its hit
            if ( digitalRead(ccw_limit_pin[i]) == 0) {
                // limit switch is hit 
                // do not move
                //printf("{\"d-%d\":\"limit_hit\"}",i);
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"b\",");
                Serial.print(i);
                Serial.println(")]");
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"m\",");
                Serial.print(i);
                Serial.println(")]");
                
                return;
            } else {
                // need to inform the software that the motor is busy
                // read by robot6_motor_driver
                b_motor_busy[i] = 1;
            }
        } else {
            // this is a sw switch or logical switch
            if ( b_ccw_limit_status[i] == 1 ) {
                //printf("{\"d-%d\":\"limit_hit\"}",i);
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"b\",");
                Serial.print(i);
                Serial.println(")]");
                Serial.print("[");
                Serial.print(i); 
                Serial.print(",(\"m\",");
                Serial.print(i);
                Serial.println(")]");
                
                return;
            } else {
                // need to inform the software that the motor is busy
                // read by robot6_motor_driver
                b_motor_busy[i] = 1;
            }
        }
        
        b_current_direction[i] =  DIRECTION_CCW;
        //#ifndef BASE_PLATE_V4
            // only for z axis perform the following 
            //if ( (i == 6) || (i == 7)) {
                // set the switchmode to sw_hard_stop and 
                // make sure its set 
                
                
                int tryCount;
                tryCount = MAX_TRY_COUNT_ST_COMM;

                while ( board[i].getSwitchMode() != SW_HARD_STOP ) {
                    board[i].hardHiZ();
                    board[i].setSwitchMode(SW_HARD_STOP); // since its a ccw direction we wanna stop when the sw is hit
                    tryCount--;
                    if ( tryCount == 0 ) {
                        break;
                    }
                    /*
                    Serial.print("board[i].getSwitchMode():");
                    Serial.println(board[i].getSwitchMode());
                    Serial.print("tryCount:");
                    Serial.println(tryCount);
                    */
                }
                if ( tryCount == 0) {
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"e\",");
                    Serial.print("\"comm_error\"");
                    Serial.println(")]");
                    /*Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"b\",");
                    Serial.print(i);
                    Serial.println(")]");
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"m\",");
                    Serial.print(i);
                    Serial.println(")]");
                    */
                    //return;
                } else {
                    Serial.print("[");
                    Serial.print(i); 
                    Serial.print(",(\"d\",");
                    Serial.print("\"sw_hard_mode\"");
                    Serial.println(")]");
                    
                }
            //}
        //#endif
        board[i].setPos(v);
        board[i].goTo(0);
        
        //printf("{\"ccw-v-%d\":\"",i);
        //Serial.print(v);
        //printf("\"}");
    }
    bool is_motor_on(int i){}
    bool is_motor_off(int i){}
    
    bool is_motor_busy(int i) { 
        return (b_motor_busy[i]); 
    }

    void print_pin_numbers(int i){
        // for given index i print all the related pins
        printf("step_pin[%d]: %d\n",i, step_pin[i]);
        printf("en_pin[%d]: %d\n",i, en_pin[i]);
        printf("dir_pin[%d]: %d\n",i, dir_pin[i]);
        printf("cw_limit_pin[%d]: %d\n",i, cw_limit_pin[i]);
        printf("ccw_limit_pin[%d]: %d\n",i, ccw_limit_pin[i]);
        printf("h_f_pin[%d]: %d\n",i, h_f_pin[i]);

    }

    void move(int direction,int motor_index,long count) {
        if ( direction == DIRECTION_CW ) {
            move_cw(motor_index,count);
        } else {
            move_ccw(motor_index,count);
        }
    }
    void run(int motor_index,int dir, float speed) {
        board[motor_index].run(dir,speed);
    }
#else 
    // followings are NOT for MODEX_STACK
    void setup_stepper() {
        int i;
        
        robot6_wire.begin(); // join i2c bus (address optional for master)
        
        // set pin Modes
        for ( i=0;i<MAX_MOTORS;i++){
        	pinMode(en_pin[i],OUTPUT);
            pinMode(dir_pin[i],OUTPUT);
            pinMode(step_pin[i],OUTPUT);
            pinMode(h_f_pin[i],OUTPUT);
            pinMode(cw_limit_pin[i],INPUT);
            pinMode(ccw_limit_pin[i],INPUT);
            
        }    
        
        // set defaults
        for ( i=0;i<MAX_MOTORS;i++){
            if ( en_pin[i] > 0 ) {
                // check if this index is used for any motor
                // 
                motor_off(i);
                max_possible_step_interval[i] = 15; //4; /* when timer reload value is 0x75 15 is good */
                initial_possible_step_interval_offset[i] = 1.5; //4; /* when timer reload value is 0x75 15 is good */
                step_interval[i] = 0.01;
                ramp_steps[i] = 50;
                current[i] = 550;
                digitalWrite(dir_pin[i],0);
                digitalWrite(h_f_pin[i],FULL_STEP); /* by default full step is active */
                b_motor_busy[i] = 0;
            }
        }
        
        #ifdef USE_TIMER
            enable_stepper_timer();
        #endif
    }
    bool is_st_comm_good(int i) {
        return 1;
    }

    #define ENABLE_STEPPER_DRIVER_INTERRUPT()  TIMSK1 |= BIT(OCIE1A)
    #define DISABLE_STEPPER_DRIVER_INTERRUPT() TIMSK1 &= ~BIT(OCIE1A)

    void enable_stepper_timer() {

        // waveform generation = 0100 = CTC
        TCCR1B &= ~BIT(WGM13);
        TCCR1B |=  BIT(WGM12);
        TCCR1A &= ~BIT(WGM11);
        TCCR1A &= ~BIT(WGM10);

        // output mode = 00 (disconnected)
        TCCR1A &= ~(3 << COM1A0);
        TCCR1A &= ~(3 << COM1B0);
        // Set the timer pre-scaler
        // Generally we use a divider of 8, resulting in a 2MHz timer
        // frequency on a 16MHz MCU. If you are going to change this, be
        // sure to regenerate speed_lookuptable.h with
        // create_speed_lookuptable.py
        TCCR1B = (TCCR1B & ~(0x07 << CS10)) | (2 << CS10);

        OCR1A = 0x4000;
        TCNT1 = 0;
        ENABLE_STEPPER_DRIVER_INTERRUPT();

    }

    void disable_stepper_timer() {
        DISABLE_STEPPER_DRIVER_INTERRUPT();
    }

    void configure_stepper_limit_switches(int i, int new_cw_limit_pin, int new_ccw_limit_pin) {
        //Serial.println("configure_stepper_limit_switches");
        cw_limit_pin[i] = new_cw_limit_pin;
        ccw_limit_pin[i] = new_ccw_limit_pin;
    }


    void set_voltage(int i, int v) {
        unsigned char value;
        /*Serial.print(i);
        Serial.print("-");
        Serial.println(v);
        */
        robot6_wire.beginTransmission(dac_address[i]);
        robot6_wire.write(0x40);
        value = v >> 4;
        robot6_wire.write(value);           // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4) 
        value = v << 4;
        robot6_wire.write(value);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x) 
        robot6_wire.endTransmission();   
        
    }

    bool is_reached(int pin) { 
        pinMode(pin,INPUT); 
        digitalWrite(pin,1);
        return( digitalRead(pin)==0 ) ;
    }

    void cw_limit_status(int i) {
        printf("{\"cw-%d\":\"",i);
        if ( is_reached(cw_limit_pin[i])) {
            printf("hit");
        } else {
            printf("not hit");
        }
        printf("\"}");
    }

    void ccw_limit_status(int i) {
        printf("{\"ccw-%d\":\"",i);
        if ( is_reached(ccw_limit_pin[i])) {
            printf("hit");
        } else {
            printf("not hit");
        }
        printf("\"}");
    }

    bool is_cw_hit(int i) {
        return (is_reached(cw_limit_pin[i]));
    }
    bool is_ccw_hit(int i) {
        return (is_reached(ccw_limit_pin[i]));
    }
    bool is_motor_busy(int i) {
        return(b_motor_busy[i]);
    }
    void set_step_interval(int i, float v) {
        //velocity - inches per second
        step_interval[i] = v;
    }
    void set_current(int i, unsigned int v) {
         current[i] = v;
    }
    void set_ramp_steps(int i, unsigned int v) {
        ramp_steps[i] = v;
    }
    void get_step_interval(int i) {
        //velocity - inches per second
        printf("{\"step_interval-%d\":",i);
        Serial.print(step_interval[i]);
        Serial.println("}");
    }
    void get_current(int i) {
        printf("{\"current-%d\":\"%d\"}",i,current[i]);
    }
    void get_ramp_steps(int i) {
        printf("{\"ramp_steps-%d\":\"%d\"}",i,ramp_steps[i]);
    }
    void get_current(int i) {
        printf("{\"current-%d\":\"%d\"}",i,current[i]);
    }
    
    void motor_on(int i) {
        digitalWrite(en_pin[i],1);
        set_voltage(i,current[i]);
        motor_state[i] = 1;
    }
    void motor_on(int i,int v) {
        digitalWrite(en_pin[i],1);
        set_voltage(i,v);
        motor_state[i] = 1;
    }

    void motor_off(int i) {

        digitalWrite(en_pin[i],0);
        set_voltage(i,0);
        motor_state[i] = 0;
        
    }
    bool is_motor_on(int i) {
        return (motor_state[i]);
    }
    bool is_motor_off(int i) {
        return (!motor_state[i]);
    }

    void calculate_pulse(int i, unsigned int step_count) {
        #ifdef SIMPLE_STEPPING
            pulse_count_stable[i] = step_count*2;
            pulse_count[i] = pulse_count_stable[i];
            current_step_interval[i] = step_interval[i];
            printf("_ptr-%u:%u\n",i, pulse_count[i]);
        #else 
            
            // calculate required number of steps for different states

            step_count = step_count * 2;
            unsigned int temp_pulse_count_stable = step_count - (ramp_steps[i]*2);

            if ( temp_pulse_count_stable < 0 ){
                pulse_count_accel[i] = step_count / 2;
                pulse_count_deaccel[i] = step_count - pulse_count_accel[i];
                pulse_count_stable[i] = 0;
                
                delay_step[i] = 0;
                current_step_interval[i] = step_interval[i];
                
            } else {
                pulse_count_accel[i] = ramp_steps[i];
                pulse_count_deaccel[i] = ramp_steps[i];
                pulse_count_stable[i] = step_count - (pulse_count_accel[i] + pulse_count_deaccel[i]);   // to avoid +/- 1 error
            
                // calculate delay_step based on required speed

                delay_step[i] = initial_possible_step_interval_offset[i] / ramp_steps[i] ;
                current_step_interval[i] = step_interval[i] + initial_possible_step_interval_offset[i];
            }

            // initial state is STATE_ACCEL
            current_state[i] = STATE_ACCEL;
            pulse_count[i] = pulse_count_accel[i];
            step_interval_changer[i] = -(delay_step[i]);    // negative change so increasing speed
                

            // for debugging
            /*
            printf("accel[%u]:%u\n",i, pulse_count_accel[i]);
            printf("stable[%u]:%u\n",i, pulse_count_stable[i]);
            printf("deaccel[%u]:%u\n",i, pulse_count_deaccel[i]);
            printf("si_changer[%d]:",i);
            Serial.println(step_interval_changer[i]);
            printf("current_si[%d]:",i);
            Serial.println(current_step_interval[i]);
            */
            //delay(500);
        #endif
    }

    void move_cw(int i,long count) {
        digitalWrite(dir_pin[i],0);
        current_limit_pin[i] = cw_limit_pin[i];
        digitalWrite(current_limit_pin[i],1);
        calculate_pulse(i,count);
        
        if ( auto_power_manage[i] == 1 ) {
            motor_on(i);
        }
        b_motor_busy[i] = 1;
    }

    void move_ccw(int i,long count) {
        digitalWrite(dir_pin[i],1);
        current_limit_pin[i] = ccw_limit_pin[i];
        digitalWrite(current_limit_pin[i],1);
        calculate_pulse(i,count);

        if( auto_power_manage[i] == 1)  {
            motor_on(i);
        }
        
        b_motor_busy[i] = 1;
    }

    void move(int direction,int motor_index,int count) {
        if ( direction == DIRECTION_CW ) {
            move_cw(motor_index,count);
        } else {
            move_ccw(motor_index,count);
        }
    }
    void set_holding_current(int i,int c) {
        holding_current[i] = c;
    
    }
    void set_accel_speed(int i,float c) {
        acc_step[i] = c;
    }
    void set_deaccel_speed(int i,float c) {
        dec_step[i] = c;
    }

    void enable_stepper_debug_info(){
        printf("[0,{\"d\":\"stepper debug info enabled\"}]");
    }
    void disable_stepper_debug_info() {
        printf("[0,{\"d\":\"stepper debug info disabled\"}]");
    }
    

    #ifdef USE_TIMER
        void stepper_task() {
            // dummy stepper task
        }
        // "The Stepper Driver Interrupt" - This timer interrupt is the workhorse.
        ISR(TIMER1_COMPA_vect) {
            b_inside_timer = 1;
    #else
        void stepper_task() {

    #endif
            int i;
            #ifdef USE_TIMER
                unsigned int timer_reload_value;
                timer_reload_value = 0x200;//0x100; // maximum possible value
                DISABLE_STEPPER_DRIVER_INTERRUPT();
            #endif
            wdt_reset();
            for (i=0;i<MAX_MOTORS;i++) {
                if ( b_motor_busy[i] == 1) {
                    #ifdef SIMPLE_STEPPING
                        if (pulse_count[i] > 0 ) {
                            timer_counter[i]++;
                            if ( timer_counter[i] >= current_step_interval[i] ) {
                                if ( digitalRead(current_limit_pin[i]) == 0) {
                                    // limit is hit
                                    // motor off if required
                                    if( auto_power_manage[i] == 1) {
                                        if ( auto_hold_torque[i] == 0) {
                                            digitalWrite(en_pin[i],0);
                                            motor_state[i] = 0;
                                        }
                                    
                                    }
                                    timer_counter[i] = 0;    
                                    b_motor_busy[i] = 0;
                                }
                                digitalWrite(step_pin[i],!digitalRead(step_pin[i]));
                                timer_counter[i] = 0;
                                pulse_count[i] = pulse_count[i] -1;
                            }
                        } else {
                            // motor off
                            if ( auto_power_manage[i] == 1) {
                                if ( auto_hold_torque[i] == 1) {
                                    if ( auto_hold_torque[i] == 0) {
                                        digitalWrite(en_pin[i],0);
                                        motor_state[i] = 0;
                                    }
                                }
                            }
                            timer_counter[i] = 0;
                            b_motor_busy[i] = 0;
                        }

                    #else 
                        if ( pulse_count[i] > 0 ) {
                            #ifdef USE_TIMER
                                timer_reload_value-=5;  // reduce the timer reload value by some constant for every active motor task that we dont see longer delays when more motor task is taken place.
                            #endif
                            timer_counter[i]++;
                            
                            if ( timer_counter[i] >= current_step_interval[i] ) {

                                if ( digitalRead(current_limit_pin[i]) == 0) {
                                    // limit is hit
                                    // motor off
                                    if ( auto_power_manage[i] == 1) {
                                        if ( auto_hold_torque[i] == 0) {
                                            digitalWrite(en_pin[i],0);
                                            motor_state[i] = 0;
                                        }
                                    }
                                    b_motor_busy[i] = 0;
                                    timer_counter[i] = 0;    
                                }
                                digitalWrite(step_pin[i],!digitalRead(step_pin[i]));
                                timer_counter[i] = 0;
                                current_step_interval[i] += step_interval_changer[i];
                                pulse_count[i]--;
                            }

                        } else {
                            // switch to next state
                            if ( current_state[i] == STATE_ACCEL ) {
                                pulse_count[i] = pulse_count_stable[i];   
                                step_interval_changer[i] = 0;   // no change
                                current_state[i] = STATE_STABLE;
                            } else if ( current_state[i] == STATE_STABLE ) {
                                pulse_count[i] = pulse_count_deaccel[i];    
                                step_interval_changer[i] = delay_step[i];   // positive change so decreasing speed
                                current_state[i] = STATE_DEACCEL;
                            } else { // ultimately if ( current_state[i] == STATE_DEACCEL ) {
                                // stop everything
                                // motor off
                                if ( auto_power_manage[i] == 1) {
                                    if ( auto_hold_torque[i] == 0) {
                                        digitalWrite(en_pin[i],0);
                                        motor_state[i] = 0;
                                    }
                                
                                }
                                b_motor_busy[i] = 0;
                                timer_counter[i] = 0;
                            }       
                            
                        } 
                    #endif
                }
            }
            
            #ifdef USE_TIMER
                //OCR1A = 0x50; // working ok 
                OCR1A = timer_reload_value; 
                //OCR1A = 0x75; // atmega with 8 serial ports
                ENABLE_STEPPER_DRIVER_INTERRUPT();
                b_inside_timer = 0;
            #endif
        }


        /*
        // testing purpose lets run non-timer based stepper drive

        void stepper_task(int i) {
            while (pulse_count[i] > 0 ) {
                wdt_reset();
                //printf("p\n");
                timer_counter[i] += 0.1;
                if ( timer_counter[i] >= current_step_interval[i] ) {
                    printf("t\n");
                
                    if ( digitalRead(current_limit_pin[i]) == 0) {
                        printf("t\n");
                
                        // limit is hit
                        // motor off
                        digitalWrite(en_pin[i],0);
                        motor_state[i] = 0;
                        timer_counter[i] = 0;  
                        b_motor_busy[i] = 0;  
                    }
                    digitalWrite(step_pin[i],!digitalRead(step_pin[i]));
                    timer_counter[i] = 0;
                    pulse_count[i] = pulse_count[i] -1;
                }
            } 
            // motor off
            motor_off(i);
        }

        void move_cw(int i, int count) {
            digitalWrite(dir_pin[i],0);
            current_limit_pin[i] = cw_limit_pin[i];
            digitalWrite(current_limit_pin[i],1);
            calculate_pulse(i,count);
            
            motor_on(i);
            stepper_task(i);
            
        }

        void move_ccw(int i, int count) {
            digitalWrite(dir_pin[i],1);
            current_limit_pin[i] = ccw_limit_pin[i];
            digitalWrite(current_limit_pin[i],1);
            calculate_pulse(i,count);
            
            motor_on(i);
            stepper_task(i);
            
        }*/


    void print_pin_numbers(int i) {
        // for given index i print all the related pins
        printf("step_pin[%d]: %d\n",i, step_pin[i]);
        printf("en_pin[%d]: %d\n",i, en_pin[i]);
        printf("dir_pin[%d]: %d\n",i, dir_pin[i]);
        printf("cw_limit_pin[%d]: %d\n",i, cw_limit_pin[i]);
        printf("ccw_limit_pin[%d]: %d\n",i, ccw_limit_pin[i]);
        printf("h_f_pin[%d]: %d\n",i, h_f_pin[i]);

    }

    #ifdef USE_MULTIPLE_TIMER

    /*
    Stepper motor index and its timers
    OC3A = TOP_X
    OC4A = TOP_Y
    OC4B = BOTTOM_X
    OC4C = BOTTOM_Y
    OC2B = Z_LEFT
    OC5A = Z_RIGHT
    OC5B = TOP_PFTRAY
    OC5C = BOTTOM_PFTRAY
    */

    void setup_timer_counters() {
        // waveform generation = 0100 = CTC
        TCCR3B &= ~BIT(WGM13);
        TCCR3B |=  BIT(WGM12);
        TCCR3A &= ~BIT(WGM11);
        TCCR3A &= ~BIT(WGM10);

        // output mode = 1 (disconnected)
        TCCR3A |= BIT(COM3A0);
        // output mode = 0 (disconnected)
        TCCR3A &= ~BIT(COM3B0);
        // Set the timer pre-scaler
        // Generally we use a divider of 8, resulting in a 2MHz timer
        // frequency on a 16MHz MCU. If you are going to change this, be
        // sure to regenerate speed_lookuptable.h with
        // create_speed_lookuptable.py
        TCCR3B = (TCCR3B & ~(0x07 << CS10)) | (2 << CS10);

        OCR3A = 0x4000;
        TCNT3 = 0;
        
        // Enable OC3A Interrupt
        TIMSK1 |= BIT(OCIE3A);
    }

    ISR(TIMER3_COMPA_vect) {
        // TOP_X

    }

    void run(int motor_index,int dir,float speed) {
        
    }
    #endif

#endif 