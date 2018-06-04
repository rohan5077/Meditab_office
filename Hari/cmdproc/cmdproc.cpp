
#include "Arduino.h"
#include <avr/wdt.h>
#include "cmdproc.h"

//#define ENABLE_DEBUG
#define MAX_NUM_OF_SERIAL_PORT 4
// there are 4 serial ports in mega 
// so we have 4 different input buffer regardless of mega or uno

// there will be bigger output_buffer for mega based devices 
// since we have more RAM and we use rs485 based addressing modes

#if defined(__AVR_ATmega2560__)
    #define OUTPUT_BUFFER_LEN 64
#else 
    #define OUTPUT_BUFFER_LEN 32
#endif

char output_buffer[OUTPUT_BUFFER_LEN];
unsigned char output_buffer_index;
char *para[16], paraCount;   
int iPara[10];
float fPara[5];

char inputBuffer[MAX_NUM_OF_SERIAL_PORT][BUFFER_LEN];

unsigned char input_buffer_index[MAX_NUM_OF_SERIAL_PORT];

int currently_processing_port_index;
unsigned int slave_address;
unsigned int board_number;
unsigned long baud_rate;
SerialPort **serial_port, *port;
unsigned long *baud;
unsigned char *port_status;
unsigned char total_ports;
unsigned char port_index;


#if defined(__AVR_ATmega2560__)
    #define addressing_mode_tx_enable_ping 41   // this is the pin which is to enable tx mode in rs485
    #define addressing_mode_serial Serial1      // this is the port which is used by Serial1
    
#else
    #define addressing_mode_tx_enable_ping 0   // this is the pin which is to enable tx mode in rs485
    #define addressing_mode_serial Serial      // this is the port which is used by Serial1
#endif 

bool b_data_came_to_addressing_mode;
bool b_data_started_coming;
const char **commands;
unsigned char total_commands;
void(**functions)(void);
void (*non_blocking_function)(void);
//void (*broadcast_receiver_function)(void);
unsigned char cmdproc_mode;

unsigned char crc_value(int i) {
    unsigned char crc;
    crc = 0;
    if (i == 0) {
        crc = '0';
    } else {
        while ( i > 0) {
            crc ^= ('0' + i%10);
            i /= 10;
        }    
    }
    return crc;
    
}

void ping() {
    printf("ok"); 
}

const char *default_commands[] = { "ping" };
void(*default_command_handlers[])(void) = { ping };
const unsigned char default_total_commands = 1;

void set_board_number(unsigned int _board_number) {
    board_number = _board_number;
    //printf("board_number: %d is set\nok\n",board_number);
}

void get_board_number(){
    printf("board_number: %d\nok\n",board_number);   
}

void default_non_blocking_function() {
}
//void default_broadcast_receiver_function() {
//}

void set_command_function_map(const char **_commands, void(**_functions)(void), unsigned char _total_commands) {
    set_command_function_map(_commands,_functions,_total_commands, default_non_blocking_function);
}

void set_command_function_map(const char **_commands, void(**_functions)(void), unsigned char _total_commands, void(*_non_blocking_function)(void)) {
    commands = _commands;
    functions = _functions;
    total_commands = _total_commands;
    non_blocking_function = _non_blocking_function;

}

/* start - command procesor related */
/* in order to use printf we need the following 2 definitions */
/* create a FILE structure to reference our UART output function */

static FILE uartout = {
    0};
/* create a output function
 This works because Serial.write, although of
 type virtual, already exists.
 */
static int uart_putchar (char c, FILE *stream) {
    wdt_reset();
    //Serial.write(c);
    if ( b_data_came_to_addressing_mode == 1) {
        // store it in output_buffer
        output_buffer[output_buffer_index] = c;
        // increment output_buffer_index
        output_buffer_index++;
        // make sure we dont exceed the limit
        output_buffer_index%=OUTPUT_BUFFER_LEN;

    } else {

        for (port_index=0;port_index<total_ports;port_index++) {

            if ( port_status[port_index] ==  PORT_STATUS_HOLDING_COMMAND ) {
                (*(serial_port[port_index])).write(c);
            }
        }
        

    }    
    return 0 ;
}
void clear_buffer(int port_index) {
    int i;
    for(i=0;i<BUFFER_LEN;i++) {
        inputBuffer[port_index][i] = 0;
    }
    input_buffer_index[port_index] = 0;

    
}


void setup_slave_address(unsigned int address ) {
    slave_address = address;
}



/////////////////////////////////////////////////////////////////////////
// Command Processing Related
/////////////////////////////////////////////////////////////////////////
unsigned char skip_prompt; /* if set to 0 then prompt ( ">" ) will be given, otherwise not given */
const char FRAME_START = '[';
const char FRAME_SEPERATOR = ',';
const char FRAME_END = ']';
const int EXPECT_START = 0;
const int EXPECT_INDEX = 1;
const int EXPECT_END = 2;

unsigned char current_port_index;
unsigned char rx_state = EXPECT_START;
char temp_data[5];
unsigned char temp_data_index;
//char **rx_data = inputBuffer;
unsigned char rx_data_index =0;


void setup_cmdproc(SerialPort *_serial_port[],unsigned long _baud_rate[],unsigned char _port_status[],unsigned char _total_ports) {
    
    setup_cmdproc(_serial_port,_baud_rate,_port_status,_total_ports, CMDPROC_MODE_NORMAL);
}

void setup_cmdproc(SerialPort *_serial_port[],unsigned long _baud_rate[],unsigned char _port_status[],unsigned char _total_ports, unsigned char mux_mode) {
    
    // total ports cannot be more than MAX_NUM_OF_SERIAL_PORT
    if (_total_ports > MAX_NUM_OF_SERIAL_PORT) {
        Serial.print("FATAL-ERROR:total_ports cannot exceed ");
        Serial.println(MAX_NUM_OF_SERIAL_PORT);
        Serial.println("application stopped now.");
        while(1);
    }

    cmdproc_mode = mux_mode;

     // immediately disable watchdog timer so set will not get interrupted
    wdt_disable();

    serial_port = _serial_port;
    baud = _baud_rate;
    port_status = _port_status;
    total_ports = _total_ports;


    currently_processing_port_index = 0;

    // initialize all the ports
    for (port_index=0;port_index<total_ports;port_index++) {
        //Serial.println((long)port);
        //Serial.println(baud[port_index]);
        //(*(serial_port[port_index])).begin(baud[port_index]);
        port_status[port_index] = PORT_STATUS_IDLE;

        
        /*(*(serial_port[port_index])).println("ready");
        (*(serial_port[port_index])).print("total_ports:");
        (*(serial_port[port_index])).println(total_ports);
        */
    }
    

    /* fill in the UART file descriptor with pointer to writer. */
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    /* The uart is the standard output device STDOUT. */
    stdout = &uartout ; 

    // enable the watchdog timer. There are a finite number of timeouts allowed (see wdt.h).
    // Notes I have seen say it is unwise to go below 250ms as you may get the WDT stuck in a
    // loop rebooting.
    // The timeouts I'm most likely to use are:
    // WDTO_1S
    // WDTO_2S
    // WDTO_4S
    // WDTO_8S

    wdt_enable(WDTO_8S);
    
    set_command_function_map(default_commands,default_command_handlers,default_total_commands);
    // printf("ready");
    rx_state = EXPECT_START;
    rx_data_index = 0;   
}

void wait_for_command() {
    
    if ( skip_prompt == 0 ) {
        printf (">");
          
    } else {
        /* we always want to prompt , so change the flag
         */
        skip_prompt = 0;
    }
    //for (port_index=0;port_index<total_ports;port_index++) {
    port_status[currently_processing_port_index] = PORT_STATUS_IDLE;
    //}
    
    clear_buffer(currently_processing_port_index);
    currently_processing_port_index = 0;
    my_read_bytes('\n');

    //Serial.print(inputBuffer);
    //Serial.println("\n");
}




bool process_rx(int p_index,char ch) {
    //Serial.println(ch);
  if ( rx_state == EXPECT_START ) {
     if ( ch == FRAME_START ) {
        rx_state = EXPECT_INDEX;
        temp_data[0] = '\0';
        temp_data_index =0 ;
     } 
  } else if ( rx_state == EXPECT_INDEX ) {
     if ( ch == FRAME_SEPERATOR ) {
        //printf("temp_data:%s",)
        temp_data[temp_data_index++] = '\0';
        current_port_index = atoi(temp_data);
        rx_state = EXPECT_END;
        inputBuffer[p_index][0] = '\0';
        rx_data_index =0 ;
     } else {
        temp_data[temp_data_index++] = ch;
     }
  } else if ( rx_state == EXPECT_END ) {
    if ( ch == FRAME_END ) {
      rx_state = EXPECT_START;
      // populate received data
      inputBuffer[p_index][rx_data_index] = '\0';
      return 1;
      
    } else {
      if ( rx_data_index < BUFFER_LEN ) {
        inputBuffer[p_index][rx_data_index++] = ch; 
      }
    }
     
  }
  return 0;
}

bool is_valid_char(char ch) {
    /*
    condition for valid char would be 
    1. it must be inbetween ascii value 32 (space) to 126 (~)
    2. or it can be a New line which is 10
    */

    // for now lets send 1 for all
    if (ch == 10) return 1;
    if ((ch >= 32) && (ch <= 126)) return 1;
    return 0;
}
void my_read_bytes(char stopChar) {
    int i[4],j; 
    char ch=0;
    
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    for ( port_index=0;port_index<total_ports;port_index++) {
        i[port_index]=0;
    }
    
    //new version of Serial.readBytesUntil(stopChar,buffer,buf_len);
    while(1) {
        
        /* a non_blocking_function which can be used to run any background task
        */
        (non_blocking_function)();
        
        
        // reset the watchdog timer
        wdt_reset();
        for ( port_index=0;port_index<total_ports;port_index++) {
            //Serial.println("0");
            if ( (*(serial_port[port_index])).available() ) {

                ch = (*(serial_port[port_index])).read();
                // check for a valid char
                if( is_valid_char(ch) == 0) {
                    
                    
                    // if its not a valid char then discard 
                    #ifdef ENABLE_DEBUG
                        Serial.print("p:");
                        Serial.println(port_index);
                        Serial.print("d:invalid ch:");
                        Serial.println((unsigned char)ch);
                        

                    #endif
                    
                    
                } else {
                    // if its a valid charactor
                    if ( cmdproc_mode == CMDPROC_MODE_MUX ) {
                        if ( process_rx(port_index,ch) ) {
                            //is_command_on_console =0;
                            port_status[port_index] = PORT_STATUS_HOLDING_COMMAND;
                            currently_processing_port_index = port_index;
                            break;
                        }
                    } else {
                        //printf("received: %d\n", ch);
                        b_data_started_coming = 1;

                        if ( ch == stopChar) {
                            b_data_started_coming = 0;
                            inputBuffer[port_index][i[port_index]] = 0;
                            i[port_index]++;
                            port_status[port_index] = PORT_STATUS_HOLDING_COMMAND;
                            currently_processing_port_index = port_index;
                            return;
                        } else if ( ch == '>') {
                            clear_buffer(port_index);
                            i[port_index]=0;
                        } else {
                            if (i[port_index] < BUFFER_LEN) {
                                inputBuffer[port_index][i[port_index]] = ch;
                                i[port_index]++;
                            } else {
                                // buffer overflow:
                                // for now just reset the index and set null string on input buffer
                                i[port_index]=0;
                                inputBuffer[port_index][i[port_index]] = 0;

                                #ifdef ENABLE_DEBUG
                                    Serial.print("port_index:");
                                    Serial.println(port_index);
                                    Serial.println("d:inputbuffer overflow");

                                #endif
                            }
                        }
                    }   // end of (cmd_proc == MUX_MODE)
                } // else of if( is_valid_char(ch) == 0)
            }
        }
        
        
    }
}

char split(char *chararr) {
    //printf ("Splitting string \"%s\" into tokens:\n",str);
    paraCount=0;
    para[paraCount] = strtok (chararr," ,");
    while (*para[paraCount] != NULL)
    {
        paraCount++;
        //printf ("%s\n",para);
        para[paraCount] = strtok (NULL, " ,");
    }
    //printf("%d\n",paraCount);
    return(paraCount);
}

char getIndexOf(char *cmd) {
    int i;
    for (i=0; i < total_commands; i++) {
        if ( strcmp(cmd,commands[i]) == 0 ) {
            return (i);
        }
    }
    return -1;
}
bool is_numeric(char *str) {

    while ( (*str) != '\0') {
        if (!(( (*str) >= '0') && ( (*str) <= '9'))) {
            return 0;
        }
        str++;
    }
    return 1;
}

void process_command() {
    unsigned char err_code;
    unsigned char target_address;
    unsigned char source_address;
    
    int cmd_index;
    unsigned char calculated_crc;
    int received_crc;   // we use int because we use -1 to denote that no crc received 
    unsigned char i,j;

    #define ERROR_NO_COMMAND       0
    #define ERROR_WRONG_ADDRESS     1
    #define ERROR_CRC           2
    #define NO_ERROR                0

    #define NO_ADDRESS      0
    #define MASTER_ADDRESS      1
    #define BROADCAST_ADDRESS   254

    b_data_came_to_addressing_mode = 0;
    if ( cmdproc_mode == CMDPROC_MODE_NORMAL ) {
        /* CMDPROC_MODE_NORMAL */

        #ifdef ENABLE_DEBUG
            Serial.print("inputBuffer[");
            Serial.print(currently_processing_port_index);
            Serial.print("]:");
            Serial.println(inputBuffer[currently_processing_port_index]);
        #endif
        err_code = split(inputBuffer[currently_processing_port_index]);
        
        
        if ( err_code == ERROR_NO_COMMAND ) {
            printf("{\"e\":\"nc\"}");
            return;
        }
        
        // check if its a address based command 
        if ( is_numeric(para[0]) ) {
            b_data_came_to_addressing_mode = 1;
            #ifdef ENABLE_DEBUG
                Serial.print("para[0]:");
                Serial.println(para[0]);
                Serial.println("d:address based command");
            #endif
            target_address = atoi(para[0]);
            // if cmd came in addressing mode we dont send the prompt, which menas the response will not have '>' at end
            skip_prompt = 1;

            if (target_address != slave_address) {
                // its a wrong address so dont do anything 
                #ifdef ENABLE_DEBUG
                    Serial.println("d:wrong address");
                #endif
                return;
            } 
            else {

                // check for crc 
                received_crc = atoi(para[paraCount-1]);

                // if crc is 0 then simply ignore 
                if ( received_crc > 0) {
                    #ifdef ENABLE_DEBUG
                        Serial.println("d:nonzero crc");
                    #endif
                    // calculate the crc of received data
                    calculated_crc = 0;
                    i = 1;  // skip the address
                    while ( i<paraCount-1 ) {
                        j = 0;
                        while( para[i][j] != '\0' ) {
                            calculated_crc ^= para[i][j];
                            j++;
                        }
                        i++;
                    }
                    #ifdef ENABLE_DEBUG
                        Serial.print("received_crc:");
                        Serial.println(received_crc);
                        Serial.print("calculated_crc:");
                        Serial.println(calculated_crc);
                    #endif
                    if ( calculated_crc !=  received_crc ) {
                        #ifdef ENABLE_DEBUG
                            Serial.println("d:crc error");
                        #endif
                        
                        return;
                    } 

                } else {
                    #ifdef ENABLE_DEBUG
                        Serial.println("d:ignore crc");
                    #endif 
                }

                
            }

            // shift all the para[x] to para[x-1]
            for ( int i=0;i<paraCount-2;i++) {
                para[i] = para[i+1];
            }
            paraCount-=2;
        } // end of 'if ( is_numeric(para[0]) ) {'
        else {
            b_data_came_to_addressing_mode = 0;
            #ifdef ENABLE_DEBUG
                Serial.println("d:no address");
            #endif
            // if target_address is 0 then we dont check for crc data at last
            target_address = NO_ADDRESS;
            source_address = NO_ADDRESS;
        }

        cmd_index = getIndexOf(para[0]);
        //Serial.println(i);    
        if ( cmd_index < 0) {
            // if the received cmd is unknown command 
            #ifdef ENABLE_DEBUG
                Serial.print("d:unknown command:");
                Serial.println(para[0]);
            #endif
            
            if (b_data_came_to_addressing_mode == 1) {
                // if we are in addressing mode and 
                // its better not to send anything 
                
            } else {
                printf("{\"e\":\"uc\"}");
            }
        } else {
            //printf("{");
            #ifdef ENABLE_DEBUG
                Serial.print("d:b_data_came_to_addressing_mode:");
                Serial.println(b_data_came_to_addressing_mode);
                Serial.print("d:cmd:");
                Serial.println(para[0]);
            #endif
            // if received cmd is not rfc then 
            // ack and 
            // reset the output buffer
            if ( strcmp(para[0],"rfc") != 0 ) {
                if ( b_data_came_to_addressing_mode == 1) {
                    // ack that we received the data 
                
                    #ifdef ENABLE_DEBUG
                        Serial.println("d:ack sent");
                    #endif
                    digitalWrite(addressing_mode_tx_enable_ping,1);     // write mode 
                    addressing_mode_serial.write('>');
                    addressing_mode_serial.print(slave_address);
                    addressing_mode_serial.print(","); // send ack
                    addressing_mode_serial.print(crc_value(slave_address));
                    addressing_mode_serial.write('\n');
                    addressing_mode_serial.flush();
                    digitalWrite(addressing_mode_tx_enable_ping,0);     // write mode 
                }
                                // but only if it came for addressing mode 
                if (b_data_came_to_addressing_mode == 1) {
                    output_buffer_index=0;
                }                
            }

            (*functions[cmd_index])();
            //printf("}");
        }
    } else {
        /* for mux mode */

        Serial.println(inputBuffer[currently_processing_port_index]);
        err_code = split(inputBuffer[currently_processing_port_index]);

        
        if ( err_code == ERROR_NO_COMMAND) {
            //printf("{\"e\":\"nc\"}");
            cp_send_event("e","nc");
            return;
        }

        cmd_index = getIndexOf(para[0]);
        //Serial.println(i);
        if ( cmd_index < 0) {
            //printf("{\"e\":\"uc\"}");
            cp_send_event("e","uc");
        } else {
            //printf("{");
            (*functions[cmd_index])();
            //printf("}");
        }
    }
}

void cp_send_event(char *e ,char *s) {
    printf("{\"%s\":\"%s\"}",e,s);
}
void cp_send_event(char *e ,int v) {
    printf("{\"%s\":\"%d\"}",e,v);
}
void cp_send_event(char *e ,float v) {
    printf("{\"%s\":\"", e); 
    for (port_index=0;port_index<total_ports;port_index++) {

        if ( port_status[port_index] ==  PORT_STATUS_HOLDING_COMMAND ) {
            (*(serial_port[port_index])).print(v);
            (*(serial_port[port_index])).println("\"}");
        }
    }
    
}

void cp_send_event(char *e ,char *s,int output_port_index) {
    port_status[output_port_index] = PORT_STATUS_HOLDING_COMMAND;
    cp_send_event(e,s);
    printf(">");
    port_status[output_port_index] = PORT_STATUS_IDLE;
}
void cp_send_event(char *e ,int v,int output_port_index) {
    port_status[output_port_index] = PORT_STATUS_HOLDING_COMMAND;
    cp_send_event(e,v);
    printf(">");
    port_status[output_port_index] = PORT_STATUS_IDLE;
}
void cp_send_event(char *e ,float v,int output_port_index) {
    port_status[output_port_index] = PORT_STATUS_HOLDING_COMMAND;
    cp_send_event(e,v);
    printf(">");
    port_status[output_port_index] = PORT_STATUS_IDLE;    
}


int get_current_port_index() {
    for (port_index=0;port_index<total_ports;port_index++) {
        if ( port_status[port_index] ==  PORT_STATUS_HOLDING_COMMAND ) {
            return(port_index);
        }
    }
}
