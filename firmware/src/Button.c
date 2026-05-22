#include <xc.h>
#include "Pin_config.h"
#include <stdbool.h>

#define Button_A1_TRIS                 TRISDbits.TRISD3
#define Button_A1_LAT                  LATDbits.LATD3
#define Button_A1_PORT                 PORTDbits.RD3
#define Button_PUPS                    TRISEbits.RDPU
#define Button_A0_TRIS                 TRISDbits.TRISD2
#define Button_A0_LAT                  LATDbits.LATD2
#define Button_A0_PORT                 PORTDbits.RD2

void Button_Initialize(unsigned char Pin_No)
{
    if (Pin_No==0)
    {
        Button_A0_PORT=0;
        Button_A0_TRIS=1;  //A0 input
        Button_PUPS=0;     //Pull ups disabled  
        
    }
    if (Pin_No==1)
    {
        Button_A1_PORT=0;
        Button_A1_TRIS=1; //A1 input
        Button_PUPS=1;     //Pull ups enabled
    }
}

char Button_Status(unsigned char Pin_No)
{
    char result;
    result=0; //Button not pressed
    if (Pin_No==0 && Button_A0_PORT==0) //When the button is not pressed, the Port is connected directly to Vcc; when pressed, it is connected to Ground
    {
        result=1; //Button pressed
    }
    if (Pin_No==1 && Button_A1_PORT==0) //When the button is not pressed, the Port is connected directly to Vcc; when pressed, it is connected to Ground
    {
        result=1; //Button pressed
    }
    return result;
}