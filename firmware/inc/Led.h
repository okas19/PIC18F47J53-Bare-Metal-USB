#include <xc.h> // include processor files - each processor file is guarded.

#define Led_A1_TRIS                 TRISAbits.TRISA1
#define Led_A1_LAT                  LATAbits.LATA1
#define Led_A1_PORT                 PORTAbits.RA1
#define Led_A1_ANS                  ANCON0bits.PCFG1
#define Led_A0_TRIS                 TRISAbits.TRISA0
#define Led_A0_LAT                  LATAbits.LATA0
#define Led_A0_PORT                 PORTAbits.RA0
#define Led_A0_ANS                  ANCON0bits.PCFG0

void Led_Initialize(unsigned char Pin_No);

void Led_0_On(void);

void Led_0_Off(void);

void Led_1_On(void);

void Led_1_Off(void);

void Led_0_Toggle(void);

void Led_1_Toggle(void);
