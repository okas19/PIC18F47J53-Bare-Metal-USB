#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>

extern volatile uint8_t usb_status_fifo[4];
extern volatile uint8_t head;  // To be written by ISR
extern uint8_t tail;           // To be read by main loop
extern volatile uint8_t count; // Number of tokens in buffer (optional) 
extern volatile bool data_sent;
extern volatile uint8_t ep0_in_toggle;
extern volatile uint8_t ep0_out_toggle;