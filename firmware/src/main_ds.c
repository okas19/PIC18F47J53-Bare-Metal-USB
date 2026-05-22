//USB Communication 

#include <xc.h> 
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include "Pin_config.h"
#include "ISR.h" 
#include "Variables.h"
#include "Led.h"
#include "usb_ds.h"
#include "Button.h"


volatile uint8_t usb_status_fifo[4];
volatile uint8_t head = 0;  // Written by ISR
uint8_t tail = 0;           // Read by main loop
volatile uint8_t count = 0; // Number of tokens in the buffer 
volatile uint8_t usb_address = 0;
volatile bool set_new_address_flag = 0;
volatile bool is_device_configured = 0;
volatile bool current_Led_A0_state = 0;
volatile bool current_Led_A1_state = 0;
volatile bool previous_Led_A0_state = 0;
volatile bool previous_Led_A1_state = 0;
volatile bool Leds_state_changed = 0;
volatile bool is_set_address_status = 0;
volatile bool data_sent = 0;


void OSCILLATOR_Initialize(void) 
{
    // SCS FOSC; IRCF 4MHz; IDLEN disabled; 
    OSCCON = 0x60;
    // INTSRC 31KHz; PLLEN enabled; TUN<5:0> 0; 
    OSCTUNE = 0x40;
    // ROSEL system_clock; ROON disabled; RODIV base_clock; ROSSLP disabled; 
    REFOCON = 0x00;
    // PRISD enabled; SOSCGO disabled; SOSCDRV enabled;   
    OSCCON2 = 0x14;
}

void main(void)  
{
    __delay_ms(100); // Wait for voltage stabilization
    PIN_Initialize();
    OSCILLATOR_Initialize();
    Led_Initialize(0);
    Led_Initialize(1);
    USB_Initialize();
    Button_Initialize(0);
    Button_Initialize(1);
    
    
    uint8_t direction = 0;
    uint8_t endpoint_no = 0;
    uint8_t pid = 0;
	uint8_t token_ustat = 0; 
    char Button_A0_pressed = 0;
     
    // TRNIF interrupt is triggered only when token packets (SETUP, IN, OUT) are received. 
    while(1)
	{
		
        while(count > 0)
		{
			token_ustat = usb_status_fifo[tail];
			tail = (tail + 1) & (4 - 1);
            
			if (((token_ustat & 0x07) >> 2) == 0)
			{
				direction = 0; // out_setup_token 
			}
			else
			{
				direction = 1; // in_token
			}
			
			endpoint_no = (token_ustat & 0x78) >> 3;
			pid = (BDT[endpoint_no * 2 + direction].Stat.Val & 0x3C) >> 2; 
            //test_A2_LAT =! test_A2_LAT;
			execute_handler(endpoint_no, direction, pid);
            //test_A2_LAT =! test_A2_LAT; 
			INTCONbits.GIE = 0;   // Temporarily disable interrupts. Atomic structure
			count--;
			INTCONbits.GIE = 1; 
			if(data_sent == 1)
            {
                check_led_status();
                
            }
			if((current_Led_A0_state != previous_Led_A0_state) || (current_Led_A1_state != previous_Led_A1_state))
			{
				Leds_state_changed = 1; 
				previous_Led_A0_state = current_Led_A0_state;
				previous_Led_A1_state = current_Led_A1_state;
			} 
		}
        if(UCONbits.SUSPND ==1 )
        {
            Led_0_On();
            Led_1_On();
        }
        Button_A0_pressed = Button_Status(0);
        if(Button_A0_pressed == 1)
        {
            USB_Remote_Wakeup_PC();
        }
	}
}