#include <xc.h>
#include "Led.h"

void Led_Initialize(unsigned char Pin_No)
{
    if (Pin_No==0)
    {
        Led_A0_TRIS=0;
        Led_A0_LAT=0; 
        
    }
    if (Pin_No==1)
    {
        Led_A1_TRIS=0;
        Led_A1_LAT=0;
    }
}

void Led_0_On()
{
    Led_A0_LAT=1;
}

void Led_0_Off()
{
    Led_A0_LAT=0;
}

void Led_1_On()
{
    Led_A1_LAT=1;
}

void Led_1_Off()
{
    Led_A1_LAT=0;
}

void Led_0_Toggle()
{
    Led_A0_LAT=!Led_A0_LAT;
}

void Led_1_Toggle()
{
    Led_A1_LAT=!Led_A1_LAT;
}