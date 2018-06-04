/*
IMPORTANT
If you are using old arduino ide and getting compilte error such as 
R_AVR_13_PCREL against undefined symbol `serialEventRun()'
Then to fix the problem please visit this page
https://github.com/TCWORLD/ATTinyCore/tree/master/PCREL%20Patch%20for%20GCC
*/

#include "Arduino.h"
#include <avr/wdt.h>
#include <TinyWireS.h> // to communicate with atmega
#include <attiny84_i2c_cmdproc.h>
#include <SoftI2CMaster.h>  // to communicate with dac
#include "robot6_attiny84_stepper_driver.h"

/*
The following definitions are kept in SoftI2CMaster.h to avoid compiler errors

#define SDA_PORT PORTA
#define SDA_PIN 2
#define SCL_PORT PORTA
#define SCL_PIN 3
#define I2C_FASTMODE 1
*/


unsigned int step_interval; // // milli seconds
unsigned int ramp_steps; // step count
unsigned int min_delay_us; //// milli seconds
unsigned int current; // milli amp
// A note about stepper pulse 
// if h_f_pin is set to 1 then its 400 pulse per revolution
// if h_f_pin is set to 0 then its 200 pulse per revolution


const int en_pin = 9; // B1
const int dir_pin = 10; //B0;
const int step_pin = 8; //B2;
const int h_f_pin = 0;  // currently this pin is a common pin to all the stepper motor and its set high by a 4.7k resistor.
const unsigned cw_limit_pin = A2;
const unsigned ccw_limit_pin = A3;

void ping() {
    printf("\"r\":\"ok\"");
}

void _non_blocking_function() {

}

const char *_commands[] = {
    "p",  // ping
    "r",  //cw limit status
    "c", // ccw limit status

    "si",  // set step interval
    "sc",   // set current
    
    "gi",  // get step interval
    "gc",   // get current
    
    "mn",  // motor on
    "mf", // motor off
    "m",  // move cw
    
    "mc", // move ccw
    "b",  // blink cw
    "bc"  // blink ccw
    
};


void(*_functions[])(void) = {
    ping,
    cw_limit_status,
    ccw_limit_status,
    
    set_step_interval,
    set_current,
    
    get_step_interval,
    get_current,
    
    motor_on,
    motor_off,
    move_cw,
    
    move_ccw,
    blink_cw,
    blink_ccw
};

/*
/////////////////////////////////////////////////////////////////////////
// I2C Master For DAC Related
/////////////////////////////////////////////////////////////////////////

#define i2c_master_init  i2c_init
void set_voltage(int v) {
  int value = v;
  int dev_address = 0x62<<1;
  i2c_start(dev_address+I2C_WRITE);
  i2c_write(0x40);
  i2c_write(value / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  i2c_write((value % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  i2c_stop();
  
}
*/
/////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////
void setup_mb(){
    set_command_function_map(_commands,_functions,14,_non_blocking_function);
    pinMode(en_pin,OUTPUT);
    pinMode(dir_pin,OUTPUT);
    pinMode(step_pin,OUTPUT);
    pinMode(h_f_pin,OUTPUT);
    pinMode(cw_limit_pin,OUTPUT);
    pinMode(ccw_limit_pin,OUTPUT);
    for (int i=0;i<3;i++){
        digitalWrite(cw_limit_pin,0);
        digitalWrite(ccw_limit_pin,0);
        delay(500);
        digitalWrite(cw_limit_pin,1);
        digitalWrite(ccw_limit_pin,1);
        delay(500);
    }
    pinMode(cw_limit_pin,INPUT);
    pinMode(ccw_limit_pin,INPUT);
    motor_off();
    digitalWrite(dir_pin,0);
    digitalWrite(h_f_pin,1);
    min_delay_us = 500;
    step_interval = 500;
    ramp_steps = 50;
    current = 1000;
    //set_voltage(current);
    

/*
    // for testing purpose
    wdt_disable();
    
    step_interval = 300;
    current = 1000;
    while ( 1) {
    
        para[1] = "1000";
        move_cw();
        //wdt_reset();
        //delay(1000);
        para[1] = "1000";
        move_ccw();
        //wdt_reset();
        
        //delay(1000);

    }
    */
}

/////////////////////////////////////////////////////////////////////////
// Stepper Driver Related
/////////////////////////////////////////////////////////////////////////

bool is_reached(int pin) { 
    pinMode(pin,INPUT); 
    digitalWrite(pin,1);
    return( digitalRead(pin)==0 ) ;
}

void cw_limit_status() {
    printf("\"cw\":\"");
    if ( is_reached(cw_limit_pin)) {
        printf("hit");
    } else {
        printf("not hit");
    }
    printf("\"");
}

void ccw_limit_status() {
    printf("\"ccw\":\"");
    if ( is_reached(ccw_limit_pin)) {
        printf("hit");
    } else {
        printf("not hit");
    }
    printf("\"");
}

void set_step_interval() {
    //velocity - inches per second
    step_interval = atoi(para[1]);
}
void set_current() {
     current = atoi(para[1]);
     //set_voltage(current);
}
void set_ramp_steps() {
    ramp_steps = atoi(para[1]);
}
void get_step_interval() {
    //velocity - inches per second
    printf("\"step_interval\":\"%d\"",step_interval);
}
void get_current() {
     printf("\"current\":\"%d\"",current);
}
void get_ramp_steps() {
    printf("\"ramp_steps\":\"%d\"",ramp_steps);
}

void motor_on() {
    //set_voltage(current);
    digitalWrite(en_pin,1);
}
void motor_off() {
    digitalWrite(en_pin,0);
    //set_voltage(0);
}
inline void delay_us(unsigned int us) {
    // since we are resetting watchdog timer inside this
    // lets adjust the us value
    us = us - 5;
    // reset the watchdog timer
    wdt_reset();
    /**
     * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
     */
    TinyWireS_stop_check();

    delayMicroseconds(us);
}
void step_motor(int d) {
    digitalWrite(step_pin,1);
    delay_us(d);
    digitalWrite(step_pin,0);
    delay_us(d);    
}
void move_motor(unsigned int step_count, unsigned int limit_pin) {
    // simple version
    unsigned int temp_steps = step_count;
    while( (temp_steps--)  && (!is_reached(limit_pin)) ) {
       step_motor(step_interval);  
    } 
}
void move_motor_v2(unsigned int step_count, unsigned int limit_pin) {
    // this accellerates and deaccellerate the speed.
    int required_steps;
    unsigned int d_us;
    unsigned int temp_steps;
    unsigned int delay_step;
    unsigned int acc_ramp_steps;
    unsigned int dec_ramp_steps;
    
    // adjust required number of steps for steady speed
    required_steps = step_count - (ramp_steps*2);

    if ( required_steps < 0 ){
        required_steps = 0;
        acc_ramp_steps = step_count/2;
        dec_ramp_steps = step_count - acc_ramp_steps;
    } else {
        acc_ramp_steps = ramp_steps;
        dec_ramp_steps = ramp_steps;
    }

    // calculate delay_step based on required speed
    // TODO
    delay_step = ( min_delay_us - step_interval) /ramp_steps;
    
    // accellerate
    temp_steps = acc_ramp_steps;
    d_us = min_delay_us;
    while( (temp_steps--)  && (!is_reached(limit_pin)) ) {
        d_us -= delay_step;
        step_motor(d_us);
    } 

    // steady
    temp_steps = required_steps;
    while( (temp_steps--)  && (!is_reached(limit_pin)) ) {
       step_motor(d_us);  
    } 
    
    // deaccelerate
    temp_steps = dec_ramp_steps;
    while( (temp_steps--)  && (!is_reached(limit_pin)) ) {
        d_us += delay_step;
        step_motor(d_us);
    } 
    

}
//////
// move clockwise
// parameters: 
// para_1: step_count
// para_2: optional , infinite loop or not , 1 - infinite, 0 - finite or no looping, helpful for testing 
//
void move_cw() {
    printf("\"r\":\"ok\"}>\n>");
    digitalWrite(dir_pin,0);

    motor_on();
    move_motor(atoi(para[1]), cw_limit_pin);
    motor_off();
}
void move_ccw() {
    printf("\"r\":\"ok\"}>\n>");
    
    digitalWrite(dir_pin,1);
    motor_on();
    move_motor(atoi(para[1]), ccw_limit_pin);
    motor_off();
}
void blink_cw() {
    printf("\"r\":\"ok\"}>\n>");
    // blink for 3 times with 500 ms delay
    pinMode(cw_limit_pin, OUTPUT);
    for (int i=0;i<2;i++) {
        digitalWrite(cw_limit_pin,1);
        delay(250);
        digitalWrite(cw_limit_pin,0);
        delay(250);
        
    }
    pinMode(cw_limit_pin,INPUT);
}
void blink_ccw() {
    printf("\"r\":\"ok\"}>\n>");
    // blink for 3 times with 500 ms delay
    pinMode(ccw_limit_pin, OUTPUT);
    for (int i=0;i<2;i++) {
        digitalWrite(ccw_limit_pin,1);
        delay(250);
        digitalWrite(ccw_limit_pin,0);
        delay(250);
        
    }
    pinMode(ccw_limit_pin,INPUT);

}
/*
//
// function test
// para[1] = step counts to repeat
// para[2] = direction, 0 for clockwise, 1 for counter clockwise
// para[3] = repeatation count , 0 means infinite, other values define the count

//
void test() {
    
    int repeatation_count = atoi(para[3]);
    if ( paraCount == 4) {
        printf("\"r\":\"ok\"}>\n>");
        while ( !repeatation_count ) {
            if ( (int)para[2] == '0' ) {
                move_cw();
            } else {
                move_ccw();
            }
            if ( repeatation_count ) {
                repeatation_count--;
            }
        }        
    } else {
        printf("\"e\":\"ip\"}>\n>");
    }

}
*/
