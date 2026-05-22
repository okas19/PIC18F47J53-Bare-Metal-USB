#include <xc.h> // include processor files - each processor file is guarded.

#define test_B2_TRIS           TRISBbits.TRISB2
#define test_B2_LAT            LATBbits.LATB2
#define test_B2_PORT           PORTBbits.RB2

#define test_B3_TRIS           TRISBbits.TRISB3
#define test_B3_LAT            LATBbits.LATB3
#define test_B3_PORT           PORTBbits.RB3

#define test_A2_TRIS          TRISAbits.TRISA2
#define test_A2_LAT           LATAbits.LATA2
#define test_A2_PORT          PORTAbits.RA2 

void PIN_Initialize(void);

void Toggle_Channel_11(void);

void Toggle_Channel_12(void);

void state_setup(void);

void state_datain(void);

void state_dataout(void);

void state_ack(void);

void state_idle(void);

void state_status(void);