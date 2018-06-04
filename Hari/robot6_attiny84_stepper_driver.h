#ifndef ROBOT6_ATTINY84_STEPPER_DRIVER
#define ROBOT6_ATTINY84_STEPPER_DRIVER

#include "Arduino.h"

void setup_mb();
// stepper driver related functions

void ping();
void cw_limit_status();
void ccw_limit_status();
void set_step_interval();
void set_current();
void set_ramp_steps();
void get_step_interval();
void get_current();
void get_ramp_steps();
void motor_on();
void motor_off();
void move_cw();
void move_ccw();
void blink_cw();
void blink_ccw();
void test();
#endif