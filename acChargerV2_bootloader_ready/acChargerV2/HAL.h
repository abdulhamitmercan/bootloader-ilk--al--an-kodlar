#ifndef HAL_H_
#define HAL_H_


#include <msp430.h>
#include "rgbLed.h"
#include "acmeter/PetitModbus.h"
#include "acmeter/PetitModbusPort.h"

#include "ux.h"
unsigned int pwmStatus;








#define TIMER0_PERIOD                               (unsigned int)(25000)
#define ACLK_PERIOD_kHz                             ((float)(32.768))
#define TIMER_TA1_PERIOD_kHz                        ((float)(1.0))

#define TIMER_TA0_PERIOD                            ((uint16_t)(1000))
#define TIMER_TA1_PERIOD                            ((uint16_t)(ACLK_PERIOD_kHz / TIMER_TA1_PERIOD_kHz))


#if HW_VERSION < 30
//-- LED OUTPUTS
#define HAL_RESET_GPIO_LEDRED()                            (P7OUT &=~ BIT3)
#define HAL_SET_GPIO_LEDRED()                              (P7OUT |= BIT3)
#define HAL_GPIO_WRITE_LEDRED(i)                           ((i == 1) ? (P7OUT |= BIT3) : (P7OUT &=~ BIT3))

#define HAL_RESET_GPIO_LEDGREEN()                          (P7OUT &=~ BIT4)
#define HAL_SET_GPIO_LEDGREEN()                            (P7OUT |= BIT4)
#define HAL_GPIO_WRITE_LEDGREEN(i)                         ((i == 1) ? (P7OUT |= BIT4) : (P7OUT &=~ BIT4))

#define HAL_RESET_GPIO_L1RELAY()                           (P7OUT &=~ BIT1)
#define HAL_SET_GPIO_L1RELAY()                             (P7OUT |= BIT1)
#define HAL_GPIO_WRITE_L1RELAY(i)                          ((i == 1) ? (P7OUT |= BIT1) : (P7OUT &=~ BIT1))

#define HAL_RESET_GPIO_L3RELAY()                           (P7OUT &=~ BIT2)
#define HAL_SET_GPIO_L3RELAY()                             (P7OUT |= BIT2)
#define HAL_GPIO_WRITE_L3RELAY(i)                          ((i == 1) ? (P7OUT |= BIT2) : (P7OUT &=~ BIT2))

#else
//-- LED OUTPUTS
#define HAL_RESET_GPIO_LEDRED()                            (P6OUT &=~ BIT7)
#define HAL_SET_GPIO_LEDRED()                              (P6OUT |= BIT7)
#define HAL_GPIO_WRITE_LEDRED(i)                           ((i == 1) ? (P6OUT |= BIT7) : (P6OUT &=~ BIT7))

#define HAL_RESET_GPIO_LEDGREEN()                          (P6OUT &=~ BIT6)
#define HAL_SET_GPIO_LEDGREEN()                            (P6OUT |= BIT6)
#define HAL_GPIO_WRITE_LEDGREEN(i)                         ((i == 1) ? (P6OUT |= BIT6) : (P6OUT &=~ BIT6))

#define HAL_RESET_GPIO_L1RELAY()                           (P6OUT &=~ BIT4)
#define HAL_SET_GPIO_L1RELAY()                             (P6OUT |= BIT4)
#define HAL_GPIO_WRITE_L1RELAY(i)                          ((i == 1) ? (P6OUT |= BIT4) : (P6OUT &=~ BIT4))

#define HAL_RESET_GPIO_L3RELAY()                           (P6OUT &=~ BIT5)
#define HAL_SET_GPIO_L3RELAY()                             (P6OUT |= BIT5)
#define HAL_GPIO_WRITE_L3RELAY(i)                          ((i == 1) ? (P6OUT |= BIT5) : (P6OUT &=~ BIT5))
#endif


//-- RELAY OUTPUTS
#define HAL_RESET_GPIO_OUTRELAY1()                         (P1OUT &=~ BIT6)
#define HAL_SET_GPIO_OUTRELAY1()                           (P1OUT |= BIT6)
#define HAL_GPIO_WRITE_OUTRELAY1(i)                        ((i == 1) ? (P1OUT |= BIT6) : (P1OUT &=~ BIT6))

//-- LOCK ACTUATOR OUTPUTS
#define HAL_RESET_GPIO_ACT_IN1()                           (P6OUT &=~ BIT1)
#define HAL_SET_GPIO_ACT_IN1()                             (P6OUT |= BIT1)
#define HAL_GPIO_WRITE_ACT_IN1(i)                          ((i == 1) ? (P6OUT |= BIT1) : (P6OUT &=~ BIT1))

#define HAL_RESET_GPIO_ACT_IN2()                           (P6OUT &=~ BIT2)
#define HAL_SET_GPIO_ACT_IN2()                             (P6OUT |= BIT2)
#define HAL_GPIO_WRITE_ACT_IN2(i)                          ((i == 1) ? (P6OUT |= BIT2) : (P6OUT &=~ BIT2))


#define HAL_RESET_GPIO_BUZZER()                            (P6OUT &=~ BIT0)
#define HAL_SET_GPIO_BUZZER()                              (P6OUT |= BIT0)
#define HAL_GPIO_WRITE_BUZZER(i)                           ((i == 1) ? (P6OUT |= BIT0) : (P6OUT &=~ BIT0))


#if HW_VERSION < 30

void HAL_GPIO_INIT_LEDRED(){
    P7DIR |= BIT3;
    P7OUT &=~ BIT3;
}

void HAL_GPIO_INIT_LEDGREEN(){
    P7DIR |= BIT4;
    P7OUT &=~ BIT4;
}

void HAL_GPIO_INIT_L1RELAY(){
    P7DIR |= BIT1;
    P7OUT &=~ BIT1;
}

void HAL_GPIO_INIT_L3RELAY(){
    P7DIR |= BIT2;
    P7OUT &=~ BIT2;
}

#else

void HAL_GPIO_INIT_LEDRED(){
    P6DIR |= BIT7;
    P6OUT &=~ BIT7;
}

void HAL_GPIO_INIT_LEDGREEN(){
    P6DIR |= BIT6;
    P6OUT &=~ BIT6;
}

void HAL_GPIO_INIT_L1RELAY(){
    P6DIR |= BIT4;
    P6OUT &=~ BIT4;
}

void HAL_GPIO_INIT_L3RELAY(){
    P6DIR |= BIT5;
    P6OUT &=~ BIT5;
}

#endif


void HAL_GPIO_INIT_ACT_IN1(){
    P6DIR |= BIT1;
    P6OUT &=~ BIT1;
}

void HAL_GPIO_INIT_ACT_IN2(){
    P6DIR |= BIT2;
    P6OUT &=~ BIT2;
}

// motor in is input for MSP430
void HAL_GPIO_INIT_MOTOR_IN(){
    P6DIR &= ~BIT3;
    //P6OUT &=~ BIT3;
}


void HAL_GPIO_INIT_BUZZER(){
    P6DIR |= BIT0;
    P6OUT &=~ BIT0;
}



void HAL_GPIO_INIT_OUTRELAY1(){
    P1DIR |= BIT6;
    P1OUT &=~ BIT6;
}



void HAL_GPIO_ACTUATOR_INIT(){
    HAL_GPIO_INIT_ACT_IN1();
    HAL_GPIO_INIT_ACT_IN2();
    HAL_GPIO_INIT_MOTOR_IN();
}


static inline void HAL_GPIO_INIT(){
    HAL_GPIO_INIT_LEDRED();
    HAL_GPIO_INIT_LEDGREEN();
    HAL_GPIO_INIT_OUTRELAY1();
    HAL_GPIO_INIT_L1RELAY();
    HAL_GPIO_INIT_L3RELAY();
    HAL_GPIO_ACTUATOR_INIT();
    HAL_GPIO_INIT_BUZZER();

}

static inline HAL_RESET_ALL_GPIO(){
    HAL_RESET_GPIO_LEDRED();
    HAL_RESET_GPIO_LEDGREEN();
    HAL_RESET_GPIO_OUTRELAY1();
    HAL_RESET_GPIO_L1RELAY();
    HAL_RESET_GPIO_L3RELAY();
    HAL_RESET_GPIO_ACT_IN1();
    HAL_RESET_GPIO_ACT_IN2();
}




void HAL_TIMER_INIT_TA2(){
    // Timer3 is using for general purpose timer isr
    TA2CCR0 = 32768;                 // Set Timer Timeout Value
    TA2CCTL0 |= CCIE;                            // Enable Overflow Interrupt
    TA2CTL |= MC_1 + TASSEL_1 + TACLR ;          // Set Mode -> Up Count, Clock -> ACLK, Clear Timer
}


void HAL_TIMER_INIT_TA1(){
    // Timer1 is using for general purpose timer isr
    TA1CCR0 = 10000-1;//5000 - 1;                 // Set Timer Timeout Value
    TA1CCTL0 |= CCIE;                            // Enable Overflow Interrupt
    TA1CTL |= MC_1 + TASSEL_2 + TACLR ;          // Set Mode -> Up Count, Clock -> ACLK, Clear Timer
}

void HAL_PWM_INIT_TA3(){
    /*
    P4DIR |= BIT7;                          // P1.0/TA1.1 output
    P4SEL0 |= BIT7;                          // Output TA1

    // Seupt TA3
    TA3CCTL0 = CCIE;                        // CCR0 interrupt enabled
    TA3CCTL0 = OUTMOD_5;
    TA3CCR0 = 12500;
    TA3CTL = TASSEL_2 | MC_1 | TACLR;       // SMCLK, contmode, clear TAR
    */


    P2DIR |= BIT2;
    P2SEL0 |= BIT2;

    // Seupt TA3
    TA0CCTL2 = CCIE;                        // CCR0 interrupt enabled
    TA0CCTL2 = OUTMOD_5;
    TA0CCR0 = TIMER0_PERIOD;
    TA0CCR2 = TIMER0_PERIOD / 2;
    TA0CTL = TASSEL_2 | MC_1 | TACLR;       // SMCLK, contmode, clear TAR

}

static inline void HAL_TIMER_INIT(){
    HAL_TIMER_INIT_TA2();
    HAL_TIMER_INIT_TA1();
    HAL_PWM_INIT_TA3();
}



void HAL_STOP_PWM3(){
    //TA3CCTL0 = OUTMOD_5;
    TA0CCTL2 = OUTMOD_5;
}

void HAL_START_PWM3(){
    //TA3CCTL0 = OUTMOD_4;
    TA0CCTL2 = OUTMOD_3;
}


void HAL_SET_PWMDUTY_PWM3(unsigned int dutyRatio){
    TA0CCR2 = (TIMER0_PERIOD / 100) * dutyRatio;
}





static inline void HAL_ADC_INIT(void){

    // Setup P2.1 for TA0.1, P1.0 output
       P2DIR |= BIT1;                                     // P2.1/TA0.1 output
       P2SEL0 |= BIT1;                                     // Output TA1 ADC trigger signal

    // Setup P1.0 A0, P1.1 A1, P1.2 A2, P1.3 A3, P1.4 A4, P1.5 A5
    // Set P1.0, P1.1, P1.2, P1.3, P1.4, P1.5 to non-IO
    P1SEL1 |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5;
//    __disable_interrupt();                        // Disable interrupts; Port Map cnfg
//    PMAPKEYID = PMAPKEY;                          // Enable access Port Mapping regs
//    P1MAP1 = PM_ANALOG;                           // Enable A1
//    P1MAP2 = PM_ANALOG;                           // Enable A2
//    PMAPKEYID = 0;                                // Disable access Port Mapping regs
//    __enable_interrupt();                         // Re-enable all interrupts

    // Setup ADC10
    ADC10CTL0 = ADC10SHT_6 | ADC10ON;                  // 16 ADCCLKs, enable ADC
    ADC10CTL1 = ADC10SHP | ADC10SHS_1 | ADC10CONSEQ_3; // Sampling timer
    // TA0.1 trigger; Repeat single ch
    ADC10CTL2 = ADC10RES;                              // 10-bit conversion results
    ADC10MCTL0 = ADC10INCH_5 | ADC10SREF_1;            // Vref+, A1

    // Setup internal reference
    while (REFCTL0 & REFGENBUSY) ;                     // If ref generator busy, WAIT
    REFCTL0 |= REFVSEL_2 | REFON;                      // Select internal ref = 2.5V
                                                       // Internal Reference ON
    __delay_cycles(7500);                                // Delay (~75us) for Ref to settle

    // Setup ADC10 timer trigger TA0.1
    TA0CCR0 = 1000 - 1;                               // PWM Period
    TA0CCR1 = 500 - 1;                                // Dutycycle TA0.1
    TA0CCTL1 = OUTMOD_4;                               // TA0CCR1 toggle
    TA0CTL = TASSEL_2 | MC_1 | TACLR;                  // ACLK, up mode, clear counter

    ADC10IE |= ADC10IE0;      // Interrupt enable
    ADC10CTL0 |= ADC10ENC;                             // Enable conversion
}



static inline void HAL_ADC_INIT2(void){
    // Setup P1.0 A0, P1.1 A1, P1.2 A2, P1.3 A3, P1.4 A4, P1.5 A5
    // Set P1.0, P1.1, P1.2, P1.3, P1.4, P1.5 to non-IO
    P1SEL1 |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5;
    P1SEL0 |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5;
//    __disable_interrupt();                        // Disable interrupts; Port Map cnfg
//    PMAPKEYID = PMAPKEY;                          // Enable access Port Mapping regs
//    P1MAP1 = PM_ANALOG;                           // Enable A1
//    P1MAP2 = PM_ANALOG;                           // Enable A2
//    PMAPKEYID = 0;                                // Disable access Port Mapping regs
//    __enable_interrupt();                         // Re-enable all interrupts

    // Setup ADC10; pulse sample mode, s/w trigger, repeat sequence of channels
    ADC10CTL0 = ADC10SHT_2 | ADC10MSC | ADC10ON;  // 16ADCclks, ADC on
    ADC10CTL1 = ADC10SHP | ADC10CONSEQ_3;         // Sampling timer, repeat seq of chs
    ADC10CTL2 |= ADC10RES;                         // 10-bit resolution
    ADC10MCTL0 = ADC10SREF_1 | ADC10INCH_5;                     // Vref = 2.5 ref, A0, A1, A2, A3, A4, A5(EOS)

    // Setup internal reference
    while (REFCTL0 & REFGENBUSY) ;                     // If ref generator busy, WAIT
    REFCTL0 |= REFVSEL_2 | REFON;                      // Select internal ref = 2.5V

    __delay_cycles(7500);

    // Setup DMA0 (ADC10IFG trigger)
    DMACTL0 = DMA0TSEL_24;                        // ADC10IFG trigger
    __data20_write_long((uintptr_t) &DMA0SA, (uintptr_t) &ADC10MEM0);
    // Source single address
    DMA0SZ = 0x06;

    /*
     * Repeated transfer
     * Increment destination byte access
     * Enable interrupt after convs
     */
    DMA0CTL = DMADT_4 | DMADSTINCR_3 | DMAEN | DMAIE;
}



static inline void HAL_SD24_INIT(void){

    SD24BCTL0 = SD24REFS | SD24SSEL_1;      // Select internal REF
                                            // Select SMCLK as SD24_B clock source

    SD24BCTL0 |= SD24PDIV_5;

    SD24BCCTL0 = SD24SCS_6;
    SD24BCCTL1 = SD24SCS_6;
    SD24BCCTL2 = SD24SCS_6;
    SD24BCCTL3 = SD24SCS_6;
    SD24BCCTL4 = SD24SCS_6;
    SD24BCCTL5 = SD24SCS_6;
    SD24BCCTL6 = SD24SCS_6;

    SD24BIE = SD24IE6;                      // Enable channel 5 interrupt

    __delay_cycles(0x3600);                 // Delay for 1.16V REF startup

    SD24BCTL1 |= SD24GRP2SC;                // Set bit to start conversion
}



static inline void HAL_SD24_INIT2(void){

    SD24BCTL0 = SD24REFS | SD24SSEL_1;      // Select internal REF
                                            // Select SMCLK as SD24_B clock source

    SD24BCCTL0 = SD24SNGL | SD24SCS_5;      // Single conversion, group 1
    SD24BCCTL1 = SD24SNGL | SD24SCS_5;      // Single conversion, group 1
    SD24BCCTL2 = SD24SNGL | SD24SCS_5;      // Single conversion, group 1

    SD24BIE |= SD24IE2;                     // Enable channel 2 interrupt
}



static inline void HAL_CLK_FREQ(void){
    /*
    UCSCTL1 |= DCORSEL_7;                   // Set RSELx for DCO = 25 MHz
    UCSCTL2 |= FLLN9;
    __delay_cycles(5000);
    */

    // Setup UCS
    UCSCTL3 |= SELREF_2;                   // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                     // Set ACLK = REFO

    __bis_SR_register(SCG0);               // Disable the FLL control loop
    UCSCTL0 = 0x0000;                      // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                   // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_1 | 761;                // Set DCO Multiplier for 8MHz
                                           // (N + 1) * FLLRef = Fdco
                                           // (243 + 1) * 32768 = 8MHz
                                           // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);               // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
    __delay_cycles(781250);

    // Loop until XT1, XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                 // Clear fault flags
    } while (SFRIFG1 & OFIFG);             // Test oscillator fault flag


}


static inline void SetVCoreUp (unsigned int level)
{
    // Open PMM registers for write access
    PMMCTL0_H = 0xA5;
    // Make sure no flags are set for iterative sequences
    __delay_cycles(5000);
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    __delay_cycles(5000);
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    __delay_cycles(5000);
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}


static inline void getAdcResults(unsigned int* ADC_Result){
    __data20_write_long((uintptr_t) &DMA0DA, (uintptr_t)(ADC_Result+0));
    // Update destination array address
    // Wait if ADC10 core is active
    while (ADC10CTL1 & ADC10BUSY) {};
    ADC10CTL0 |= ADC10ENC | ADC10SC;      // Sampling and conversion ready
    __no_operation();                         // BREAKPOINT; check ADC_Result
}


void HAL_INIT_I2C(unsigned int slaveAddr)
{
    //I2C Pins for LM75
    P2SEL0 |= BIT5 | BIT6;      // Set P2.5,P2.6 to UCB0SCL, UCB0SDA

    // Setup eUSCI_B0
    UCB0CTLW0 |= UCSWRST;                             // Enable SW reset
    UCB0CTLW0 |= UCMST | UCMODE_3 | UCSYNC | UCSSEL_2;// I2C Master, use SMCLK

    UCB0BRW_L = 160;                                  // fSCL = SMCLK/160 = ~100kHz
    UCB0BRW_H = 0;
    UCB0I2CSA = slaveAddr;                           // Slave Address is 048h
    UCB0CTLW0 &= ~UCSWRST;                            // Clear SW reset, resume operation
    UCB0IE |= UCNACKIE;
}

void HAL_INIT_UART()
{
    // Setup P3.0 UCA0RXD, P3.1 UCA0TXD
    P3SEL0 |= BIT0 | BIT1;                   // Set P3.0, P3.1 to non-IO
    P3DIR |= BIT0 | BIT1;                   // Enable UCA0RXD, UCA0TXD

    // Setup eUSCI_A0
    UCA0CTLW0 |= UCSWRST;                   // **Put state machine in reset**
    UCA0CTLW0 |= UCSSEL_2;                  // CLK = ACLK
    //UCA0BRW_L = 0x03;                       // 32kHz/9600=3.41 (see User's Guide)
    //UCA0BRW_H = 0x00;                       //
    UCA0BRW = 2604;//217;
    UCA0MCTLW = 0x5300;                     // Modulation UCBRSx=0x53, UCBRFx=0
    UCA0CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
}

void HAL_INIT_XINT(){
    P2DIR &= ~BIT3;
    P2OUT =  0x08;
    P2REN |= 0x08;
    P2IE |= 0x08;
    P2IES |= 0x08;
    P2IFG &= ~0x08;
}


static inline void HAL_INIT(void){
    HAL_CLK_FREQ();
    HAL_GPIO_INIT();
    HAL_TIMER_INIT();
    HAL_ADC_INIT2();
    HAL_SD24_INIT();
    HAL_INIT_I2C(0x48);
    HAL_INIT_UART();
    //HAL_INIT_XINT();

};

void HAL_PORT_MAPPING(){
    P4DIR |= BIT2;
    P4SEL0 |= BIT2;

    P4DIR |= BIT1;
    P4SEL0 |= BIT1;

    volatile unsigned char *ptr1, *ptr2;

    P2SEL0 |= BIT1;

    volatile unsigned char *ptr;

    PMAPKEYID = PMAPKEY;                    // Enable access Port Mapping regs

#ifdef PORT_MAP_RECFG
    PMAPCTL = PMAPRECFG;                    // Allow reconfig during runtime
#endif

    ptr1 = &P4MAP2;
    *ptr1 = PM_UCA3RXD;

    ptr2 = &P4MAP1;
    *ptr2 = PM_UCA3TXD;


    ptr = &P2MAP1;
    *ptr = PM_UCB1SIMO;


    PMAPKEYID = 0;                          // Disable access Port Mapping regs
}

static inline void HAL_SYS_INIT(){
    //WDTCTL = WDTPW + WDTHOLD;           // Stop watchdog timer
    SetVCoreUp(3);
    __delay_cycles(5000);
    HAL_INIT();
    HAL_RESET_ALL_GPIO();
    __delay_cycles(50000);
    __bis_SR_register(GIE);

    /*WDTCTL = WDTPW + WDTHOLD;    // Watchdog stop

    SetVCoreUp(3);               // Core voltage

    HAL_CLK_FREQ();              // Saat kayna��n� ayarla

    HAL_PORT_MAPPING();          // Port y�nlendirmeleri

    HAL_GPIO_INIT();             // GPIO'lar� ayarla

    HAL_TIMER_INIT();            // Timer ba�lat

    HAL_ADC_INIT2();             // ADC init

    HAL_SD24_INIT();             // SD24 init

    HAL_INIT_UART();             // UART

    HAL_INIT_I2C(0x48);          // I2C

    HAL_INIT_XINT();             // Harici kesme

    HAL_RESET_ALL_GPIO();        // GPIO�lar� s�f�rla

    __delay_cycles(50000);       // Stabilite i�in bekleme

    // �imdi �st katmanlar� ba�lat
    rgbLed_init();               // SPI, RGB LED
    InitPetitModbus(1);          // Modbus stack

    __bis_SR_register(GIE);      // Global interrupt enable*/


}






void PwmStartStop(unsigned int pwmStart){
    if(pwmStart == 1){
        pwmStart = 0;
        pwmStatus = 1;
        HAL_START_PWM3();
    }
    else if(pwmStart == 0){
        HAL_STOP_PWM3();
        pwmStatus = 0;
        pwmStart = 0;
    }

}





#endif /* HAL_H_ */