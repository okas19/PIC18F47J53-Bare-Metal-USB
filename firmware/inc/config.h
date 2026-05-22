#include <xc.h>

// CONFIG1L
#pragma config WDTEN = OFF    // Watchdog Timer->Disabled - Controlled by SWDTEN bit
#pragma config PLLDIV = 4    // PLL Prescaler Selection->100 (16 MHz oscillator input Divider 4 PLL input 4 MHz)
#pragma config CFGPLLEN = OFF    // PLL Enable Configuration Bit->PLL Enabled (Active Low!)
#pragma config STVREN = ON    // Stack Overflow/Underflow Reset->Enabled
#pragma config XINST = OFF    // Extended Instruction Set->Enabled

// CONFIG1H
#pragma config CPUDIV = OSC3_PLL3    // CPU System Clock Postscaler->CPU system clock divide by 3    (Since PLL is enabled)
#pragma config CP0 = OFF    // Code Protect->Program memory is not code-protected

// CONFIG2L
#pragma config OSC = HSPLL    // Oscillator->HSPLL, USB-HS  (FOSC2 bit is selected as 1. Required value for USB is 1)
#pragma config SOSCSEL = DIG    // T1OSC/SOSC Power Selection Bits->Digital (SCLKI) mode selected
#pragma config CLKOEC = ON    // EC Clock Out Enable Bit ->CLKO output enabled on the RA6 pin
#pragma config FCMEN = ON    // Fail-Safe Clock Monitor->Enabled
#pragma config IESO = ON    // Internal External Oscillator Switch Over Mode->Enabled

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler->1:32768

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF    // DSWDT Clock Select->DSWDT uses INTRC
#pragma config RTCOSC = T1OSCREF    // (T1OSCREF) RTCC Clock Select->RTCC uses T1OSC/T1CKI      // (INTOSCREF)    // RTCC Clock Select->RTCC uses INTRC
#pragma config DSBOREN = ON    // Deep Sleep BOR->Enabled
#pragma config DSWDTEN = ON    // Deep Sleep Watchdog Timer->Enabled
#pragma config DSWDTPS = G2    // Deep Sleep Watchdog Postscaler->1:2,147,483,648 (25.7 days)

// CONFIG3H
#pragma config IOL1WAY = ON    // IOLOCK One-Way Set Enable bit->The IOLOCK bit (PPSCON<0>) can be set once
#pragma config ADCSEL = BIT10    // ADC 10 or 12 Bit Select->10 - Bit ADC Enabled
#pragma config MSSP7B_EN = MSK7    // MSSP address masking->7 Bit address masking mode

// CONFIG4L
#pragma config WPFP = PAGE_127    // Write/Erase Protect Page Start/End Location->Write Protect Program Flash Page 127
#pragma config WPCFG = OFF    // Write/Erase Protect Configuration Region ->Configuration Words page not erase/write-protected

// CONFIG4H
#pragma config WPDIS = OFF    // Write Protect Disable bit->WPFP<6:0>/WPEND region ignored
#pragma config WPEND = PAGE_WPFP    // Write/Erase Protect Region Select bit (valid when WPDIS = 0)->Pages WPFP<6:0> through Configuration Words erase/write protected
#pragma config LS48MHZ = SYS48X8    // Low Speed USB mode with 48 MHz system clock bit->System clock at 48 MHz USB CLKEN divide-by is set to 8

#define _XTAL_FREQ 16000000
#define INPUT   1
#define OUTPUT  0
#define HIGH    1
#define LOW     0
#define ANALOG      1
#define DIGITAL     0