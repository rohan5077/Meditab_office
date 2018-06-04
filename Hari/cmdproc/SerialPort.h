#ifndef serial_port_h
#define serial_port_h
#include <Stream.h>
class SerialPort : public Stream
{
    /*
private:

  // static data
  static char _receive_buffer[_SS_MAX_RX_BUFF]; 
  static volatile uint8_t _receive_buffer_tail;
  static volatile uint8_t _receive_buffer_head;
  static char _transmit_buffer[_SS_MAX_TX_BUFF]; 
  static volatile uint8_t _transmit_buffer_tail;
  static volatile uint8_t _transmit_buffer_head;
  static SerialPort *active_object;

  static void receiveEvent(unsigned char len);
  static void requestEvent();
  
*/
public:
  // public methods
  SerialPort();
  ~SerialPort();
  virtual void begin(long baud);
  virtual void begin(long baud,int i);
  virtual void end();
  virtual void update();
  virtual int peek();

  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
  operator bool() { return true; }
  
  using Print::write;

};
#endif