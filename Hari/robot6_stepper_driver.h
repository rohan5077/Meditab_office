#ifndef ROBOT6_STEPPER_DRIVER
#define ROBOT6_STEPPER_DRIVER

/*
PLEASE NOTE:

IF YOU ARE COMPILING FOR ROBOT6, DISABLE THE MACTRO #define BASE_PLATE_V4 BELOW
IF YOU ARE COMPILING FOR CONVEYOUR SYSTEM, ENABLE THE MACRO #define BASE_PLATE_V4

*/

//#define BASE_PLATE_V4

#define USE_L6470_SHIELD

#ifdef USE_L6470_SHIELD
#define USE_MODEX_STACK
#endif

#define USE_L6470_SW    0
#define NO_PIN -1

#ifdef USE_MODEX_STACK
    #include <SparkFunAutoDriver.h>
#endif
#define MAX_TRY_COUNT_ST_COMM 10
        

#ifdef USE_MODEX_STACK
    extern long motor_position[];
    #ifdef BASE_PLATE_V4
        
        extern const int slave_cs_0_pin; // 22;
        extern const int slave_cs_1_pin; // 23;
        extern const int slave_cs_2_pin; // 24;
        extern const int slave_cs_3_pin; // 25;
        extern const int slave_cs_4_pin; // 26;
        extern const int slave_cs_5_pin; // 27;
        extern const int slave_cs_6_pin; // 28;
        extern const int slave_cs_7_pin; // 29;
        extern const int slave_cs_8_pin; // 30;
        extern const int slave_cs_9_pin; // 31;
        
        extern const int spi_reset_0_pin; // 32;
        extern const int spi_reset_1_pin; // 33;
        extern const int spi_reset_2_pin; // 34;
        extern const int spi_reset_3_pin; // 35;
        extern const int spi_reset_4_pin; // 36;
        extern const int spi_reset_5_pin; // 37;
        extern const int spi_reset_6_pin; // 38;
        extern const int spi_reset_7_pin; // 39;
        extern const int spi_reset_8_pin; // 40;
        extern const int spi_reset_9_pin; // 41;
        
        /*
        const int _slave_cs_0_pin = 22;
        const int _slave_cs_1_pin = 23;
        const int _slave_cs_2_pin = 24;
        const int _slave_cs_3_pin = 25;
        const int _slave_cs_4_pin = 26;
        
        const int _spi_reset_0_pin = 32;
        const int _spi_reset_1_pin = 33;
        const int _spi_reset_2_pin = 34;
        const int _spi_reset_3_pin = 35;
        const int _spi_reset_4_pin = 36;
        */
        #define  MAX_MOTORS 10   // this is maximum supported motors

    #else
        const int _spi_reset_pin = 47;
        const int _xt_cs_pin = 22;
        const int _yt_cs_pin = 23;
        const int _xb_cs_pin = 24;
        const int _yb_cs_pin = 25;
        const int _pft_cs_pin = 26;
        const int _pfb_cs_pin = 27;
        const int _zl_cs_pin = 28;
        const int _zr_cs_pin = 29;

        #define  MAX_MOTORS 8 // this is maximum supported motors   
    #endif
    
    
#else
    #define  MAX_MOTORS 8 // this is maximum supported motors   
#endif






extern const int en_pin[];
extern const int dir_pin[];
extern const int step_pin[];
extern const int h_f_pin[];
extern bool b_motor_busy[];
extern bool auto_power_manage[];
extern bool auto_hold_torque[];
extern int cw_limit_pin[];
extern int ccw_limit_pin[];
extern const int dac_address[];
extern float step_interval[]; // // sudo unit
extern unsigned int current[]; // milli amp
extern bool b_inside_timer;
extern unsigned int holding_current[];
extern float acc_step[]; // // sudo unit
extern float dec_step[]; // // sudo unit
extern float power_kval[];  // sudo unit

extern bool b_monitor_dependent_motor[];   // 1 - monitor dependentancy, 0 - dont 
extern unsigned char dependent_motor_index[]; // motor index of the dependent axis
extern bool dependent_motor_assert_direction[];  // ccl - 0 cl - 1

#define USE_TIMER
//#define DO_NOT_USE_TIMER
//#define USE_MULTIPLE_TIMER

#define SMOOTH_STEPPING
//#define SIMPLE_STEPPING




void setup_stepper();
void enable_stepper_debug_info();
void disable_stepper_debug_info();

bool is_st_comm_good(int);
void get_sw_mode(int);
void set_sw_mode(int,unsigned int);

void stepper_task();
void enable_stepper_timer();
void disable_stepper_timer();

void configure_stepper_limit_switches(int,int,int);
void cw_limit_status(int);
void ccw_limit_status(int);
bool is_cw_hit(int);
bool is_ccw_hit(int);
void set_step_interval(int,float);
void set_current(int,unsigned int);
void set_ramp_steps(int,unsigned int);
void get_step_interval(int);
void get_current(int);
void get_ramp_steps(int);
void set_voltage(int,int);
void set_holding_current(int,int);
void set_accel_speed(int,float);
void set_deaccel_speed(int,float);
void set_stepper_power(int ,float);// set stepper power for acc,dacc and run

void motor_on(int);
void motor_on(int,int);
void motor_off(int);
void move_cw(int,long);
void move_ccw(int,long);
bool is_motor_on(int);
bool is_motor_off(int);
bool is_motor_busy(int);
void print_pin_numbers(int i);
void run(int,int,float);
// direction contants useful for batch moves 
const int DIRECTION_CW = 0;
const int DIRECTION_CCW = 1;
void move(int direction,int motor_index,long count);

#endif