#include "SerialPort.h"
#include "Arduino.h"
// dummy declaration of a class SerialPort
// this way we can typecast any serial port

//
// Statics
//
/*
SerialPort *SerialPort::active_object = 0;
// buffer for reception
char SerialPort::_receive_buffer[_SS_MAX_RX_BUFF]; 
volatile uint8_t SerialPort::_receive_buffer_tail = 0;
volatile uint8_t SerialPort::_receive_buffer_head = 0;
// buffer for transmission
char SerialPort::_transmit_buffer[_SS_MAX_TX_BUFF]; 
volatile uint8_t SerialPort::_transmit_buffer_tail = 0;
volatile uint8_t SerialPort::_transmit_buffer_head = 0;


//
// Private methods
//

//
// The receive routine called by the interrupt handler
//
void SerialPort::receiveEvent(unsigned char len)
{
 
}

void SerialPort::requestEvent() {
 
}
*/
//
// Constructor
//
SerialPort::SerialPort() 
{
 
}

//
// Destructor
//
SerialPort::~SerialPort()
{
  
}

//
// Public methods
//
/*
void SerialPort::begin(long baud)
{
  //Serial.begin(baud);
}

void SerialPort::begin(long baud,int i)
{
 
}


void SerialPort::end()
{
  
}

void SerialPort::update()
{

}

// Read data from buffer
int SerialPort::read()
{

}

int SerialPort::available()
{

}

size_t SerialPort::write(uint8_t b)
{
    Serial.println("write");
}

void SerialPort::flush()
{
  // There is no tx buffering, simply return
}

int SerialPort::peek()
{
 
}
*/
