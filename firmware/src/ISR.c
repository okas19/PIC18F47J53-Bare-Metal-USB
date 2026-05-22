#include <xc.h>
#include <pic18f47j53.h>
#include "ISR.h"
#include "Pin_config.h"
#include "Variables.h"
#include "usb_ds.h"  
#include "Led.h" 
#include "config.h"

void __interrupt() INTERRUPT_InterruptManager (void)  
{
    if (PIE2bits.USBIE == 1 && PIR2bits.USBIF == 1) 
    {
        if(UIEbits.TRNIE == 1 && UIRbits.TRNIF == 1)
        {
            if (count < 4)
            {
                usb_status_fifo[head] = USTAT; 
                head = (head + 1) & (4 - 1); 
                count++;
            }
            UIRbits.TRNIF = 0;
        }
        if(UIEbits.URSTIE == 1 && UIRbits.URSTIF == 1)
        {
            UADDR = 0; // SIE sets it to zero automatically!
            UEP0 = 0x16; // EPHSHK 1  EPCONDIS 0  EPOUTEN 1  EPINEN 1 EPSTALL 0
            ep0_in_toggle = 1;
            ep0_out_toggle = 1;
            BDT[0].ADRL = 0x00;  
            BDT[0].ADRH = 0x05;  
            BDT[0].CNT = 64;
            BDT[0].Stat.Val = 0xC8; // Give ownership to SIE
            UCONbits.PKTDIS = 0;
            UIRbits.URSTIF = 0;
        }
        if(UIEbits.STALLIE == 1 && UIRbits.STALLIF == 1)
        {
            if(UEP0bits.EPSTALL0 == 1)
            {
                UEP0bits.EPSTALL0 = 0;
            }
            UIRbits.STALLIF = 0;
        } 
        
        if(UIEbits.UERRIE == 1 && UIRbits.UERRIF == 1 )
        {
            UEIR = 0;
            UIRbits.UERRIF = 0; //Read-only!
        }
        
        
        if(UIEbits.SOFIE == 1 && UIRbits.SOFIF == 1)
        {
            UIRbits.SOFIF = 0;
        }
        
        
        if(UIEbits.IDLEIE == 1 && UIRbits.IDLEIF == 1)
        {
            // Enable power saving by suspending the internal clock of the USB module
            UCONbits.SUSPND = 1; 

            // Enable the Activity interrupt to wake up if any activity occurs while idle
            UIRbits.ACTVIF = 0; // Clear flag
            UIEbits.ACTVIE = 1; // Enable Activity interrupt

            // Temporarily disable Idle interrupt (since we are already suspended)
            UIEbits.IDLEIE = 0; 
            UIRbits.IDLEIF = 0; // Clear interrupt flag
        }
        
        
        if(UIEbits.ACTVIE == 1 && UIRbits.ACTVIF == 1)
        {
            if (UCONbits.SUSPND == 1)
            {
                UCONbits.SUSPND = 0;
                // Short software delay to ensure the hardware (SIE) does not miss the wake-up (Resume) signals
                // Note: Using delays inside an ISR is normally not recommended, but this is an exceptional necessity for USB wake-up clock synchronization.
                __delay_ms(2); 
            }

            // Re-enable Idle interrupt in case the device enters suspend state again
            UIRbits.IDLEIF = 0;
            UIEbits.IDLEIE = 1;

            // Disable Activity interrupt (since we have already woken up)
            UIEbits.ACTVIE = 0;
            UIRbits.ACTVIF = 0; // Clear interrupt flag
        }
    }
    UIRbits.SOFIF = 0;
    UIRbits.IDLEIF = 0;
    PIR2bits.USBIF = 0; 
}