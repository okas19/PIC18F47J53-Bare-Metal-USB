#include "Pin_config.h"
#include "xc.h"

void PIN_Initialize(void) 
{
    LATE = 0x00;
    LATD = 0x00;
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
 
    TRISE = 0x00;
    TRISA = 0x00;
    TRISB = 0x00;                    
    TRISC = 0x00;
    TRISD = 0x00;

    ANCON0 = 0xFF;  // All pins are set as digital
    ANCON1 = 0xFF;  // All pins are set as digital
    
    test_A2_TRIS = 0;
    test_A2_LAT = 0;
    
    test_B2_TRIS = 0;
    test_B2_LAT = 0;
    
    test_B3_TRIS = 0;
    test_B3_LAT = 0;
      
    TRISEbits.REPU = 1; //PORTE Pull-ups are enabled 
        
    ODCON1 = 0x00;
    ODCON2 = 0x00;
    ODCON3 = 0x00;  
        
    RCONbits.IPEN = 0;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void Toggle_Channel_11(void)
{
    test_A2_LAT =! test_A2_LAT;
}

void Toggle_Channel_12(void)
{
    test_B2_LAT =! test_B2_LAT;
}

void state_setup(void)
{
    test_A2_LAT = 1;
    test_B2_LAT = 0;
    test_B3_LAT = 1;
}

void state_datain(void)
{
    test_A2_LAT = 0;
    test_B2_LAT = 1;
    test_B3_LAT = 1;
}

void state_dataout(void)
{
    test_A2_LAT = 1;
    test_B2_LAT = 1;
    test_B3_LAT = 0;
}

void state_ack(void)
{
    test_A2_LAT = 0;
    test_B2_LAT = 1;
    test_B3_LAT = 0;
}

void state_idle(void)
{
    test_A2_LAT = 1;
    test_B2_LAT = 1;
    test_B3_LAT = 1;
}

void state_status(void)
{
    test_A2_LAT = 1;
    test_B2_LAT = 0;
    test_B3_LAT = 0;
}