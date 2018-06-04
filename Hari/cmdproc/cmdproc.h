#ifndef CMDPROC_H
#define CMDPROC_H
#include <avr/wdt.h>
#include "SerialPort.h"
#include "Arduino.h"
extern unsigned int slave_address;
extern unsigned int board_number;
extern unsigned long baud_rate;
extern char *para[], paraCount;
extern int iPara[];
extern float fPara[];
extern unsigned char skip_prompt;

extern char output_buffer[];
extern unsigned char output_buffer_index;
extern bool b_data_came_to_addressing_mode;;
extern unsigned char *port_status;
extern unsigned char total_ports;
extern SerialPort **serial_port, *port;

#define PORT_STATUS_IDLE      0
#define PORT_STATUS_HOLDING_COMMAND 1

// cmd proc modes 
#define CMDPROC_MODE_NORMAL     0
#define CMDPROC_MODE_MUX        1

extern unsigned char current_port_index;
/*currently we support upto 64 bytes in a command */
#define BUFFER_LEN 64
void clear_buffer();
void setup_slave_address(unsigned int);
void set_board_number(unsigned int);
void get_board_number();
void setup_cmdproc(SerialPort **_serial_port,unsigned long *_baud_rate,unsigned char *_port_status,unsigned char _total_ports);
void setup_cmdproc(SerialPort **_serial_port,unsigned long *_baud_rate,unsigned char *_port_status,unsigned char _total_ports,unsigned char);
void wait_for_command();
void process_command();
void my_read_bytes(char stopChar);
void set_command_function_map(const char **_commands, void(**_functions)(void),unsigned char _total_commands);
void set_command_function_map(const char **_commands, void(**_functions)(void),unsigned char _total_commands,void(*_non_blocking_function)(void));
int get_current_port_index();
void cp_send_event(char * ,char *);
void cp_send_event(char * ,int);
void cp_send_event(char * ,float);
void cp_send_event(char * ,char *,int);
void cp_send_event(char * ,int,int);
void cp_send_event(char * ,float,int);


#endif