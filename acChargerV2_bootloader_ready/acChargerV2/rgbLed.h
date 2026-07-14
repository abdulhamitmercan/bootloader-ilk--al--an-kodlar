#ifndef RGBLED_H_
#define RGBLED_H_

#include <msp430.h>
#include "ws2812/ws2812.h"

#define NUMBEROFLEDS    60
#define ENCODING        3       // possible values 3 and 4




uint8_t update;
ledcolor_t blankLed = {0x00, 0x00, 0x00};
uint8_t colorIdx;
ledcolor_t led;

// buffer to store encoded transport data
uint8_t frameBuffer[(ENCODING * sizeof(ledcolor_t) * NUMBEROFLEDS)] = { 0, };

// array with 15 rbg colors
ledcolor_t leds[NUMBEROFLEDS] = {
/*
        // rainbow colors
        { 0xFF, 0x00, 0x00 },
        { 0xFF, 0x3F, 0x00 },
        { 0xFF, 0x7F, 0x00 },
        { 0xFF, 0xFF, 0x00 },
        { 0x00, 0xFF, 0x00 },
        { 0x00, 0xFF, 0x3F },
        { 0x00, 0xFF, 0x7F },
        { 0x00, 0xFF, 0xFF },
        { 0x00, 0x00, 0xFF },
        { 0x3F, 0x00, 0xFF },
        { 0x7F, 0x00, 0xFF },
        { 0xFF, 0x00, 0xFF },
        { 0xFF, 0x00, 0x7F },
        { 0xFF, 0x00, 0x3F },
        { 0xFF, 0x00, 0x0F },
*/
        // rainbow colors
        { 0xFF, 0x00, 0x00 },
        { 0xFF, 0x3F, 0x00 },
        { 0xFF, 0x7F, 0x00 },
        { 0xFF, 0xFF, 0x00 },
        { 0x00, 0xFF, 0x00 },
        { 0x00, 0xFF, 0x3F },
        { 0x00, 0xFF, 0x7F },
        { 0x00, 0xFF, 0xFF },
        { 0x00, 0x00, 0xFF },
        { 0x3F, 0x00, 0xFF },
        { 0x7F, 0x00, 0xFF },
        { 0xFF, 0x00, 0xFF },
        { 0xFF, 0x00, 0x7F },
        { 0xFF, 0x00, 0x3F },
        { 0xFF, 0x00, 0x0F },
};





typedef struct {
    uint16_t run;
    uint16_t conn;
    uint16_t charge;
} t_colorObj;

t_colorObj rgbledObj;
t_colorObj rgbledObjOld;




void shiftLed(ledcolor_t* leds, ledcount_t ledCount) {
    ledcolor_t tmpLed;
    ledcount_t ledIdx;

    tmpLed = leds[ledCount-1];
    for(ledIdx=(ledCount-1); ledIdx > 0; ledIdx--) {
        leds[ledIdx] = leds[ledIdx-1];
    }
    leds[0] = tmpLed;
}

// copy bytes from the buffer to SPI transmit register
// should be reworked to use DMA
void sendBuffer(uint8_t* buffer, ledcount_t ledCount) {
    uint16_t bufferIdx;
    for (bufferIdx=0; bufferIdx < (ENCODING * sizeof(ledcolor_t) * ledCount); bufferIdx++) {
        while (!(UCB1IFG & UCTXIFG));       // wait for TX buffer to be ready
        UCB1TXBUF_L = buffer[bufferIdx];
    }
    //__delay_cycles(300);
}

void sendBufferDma(uint8_t* buffer, ledcount_t ledCount) {
    DMA2SA = (__SFR_FARPTR) buffer;     // source address
    DMA2DA = (__SFR_FARPTR) &UCB1TXBUF_L;   // destination address
    // single transfer, source increment, source and destination byte access, interrupt enable
    DMACTL1 = DMA2TSEL__USCIB1TX;
    DMA2SZ = (ENCODING * sizeof(ledcolor_t) * ledCount);
    DMA2CTL = DMADT_0 | DMASRCINCR_3 | DMASRCBYTE | DMADSTBYTE | DMAEN; //| DMAIE;

    //start DMA
    UCB1IFG ^= UCTXIFG;
    UCB1IFG ^= UCTXIFG;
}

// configure MCLK and SMCLK to be sourced by DCO with a frequency of
//   8Mhz (3-bit encoding)
// 6.7MHz (4-bit encoding)
void configClock(void) {
    /*
    CSCTL0_H = 0xA5;
#if ENCODING == 3
    CSCTL1 = DCOFSEL_3;            // DCO frequency setting = 8MHz
#else
    CSCTL1 = DCOFSEL_2;            // DCO frequency setting = 6.7MHz
#endif
    CSCTL2 = SELS__DCOCLK + SELM__DCOCLK;
    CSCTL3 = DIVS__1 + DIVM__1;
    */

}

void Port_Mapping(void)
{
    /*
    //P2DIR |= BIT1;
    P2SEL0 |= BIT1;

    volatile unsigned char *ptr;

    PMAPKEYID = PMAPKEY;                    // Enable access Port Mapping regs

#ifdef PORT_MAP_RECFG
    PMAPCTL = PMAPRECFG;                    // Allow reconfig during runtime
#endif

    ptr = &P2MAP1;

    *ptr = PM_UCB1SIMO;


    PMAPKEYID = 0;                          // Disable access Port Mapping regs
    */
}

void configSPI(void) {
    UCB1CTLW0 |= UCSWRST;                      // **Put state machine in reset**
    UCB1CTLW0 |= UCMST + UCSYNC + UCCKPL + UCMSB;   // 3-pin, 8-bit SPI master
                                                    // Clock polarity high, MSB
    UCB1CTLW0 |= UCSSEL__SMCLK;                     // SMCLK

#if ENCODING == 3
    UCB1BR0 = 9;        // SPICLK 8MHz/3 = 2.66MHz
    UCB1BR1 = 0;
#else
    UCB1BR0 = 2;        // SPICLK 6.7MHz/2 = 3.35MHz
    UCB1BR1 = 0;
#endif
    //UCB1MCTLW = 0;                            // No modulation
    UCB1CTLW0 &= ~UCSWRST;

    // UCB0SIMO = LED data signal on P2.0
    //P2SEL0 &= ~BIT0;
    //P2SEL1 |= BIT0;
}




void rgbLed_init(){
    Port_Mapping();
    configClock();
    configSPI();

}




void rgbLed_demo(){
    // blank all LEDs

    fillFrameBufferSingleColor(&blankLed, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);
    //__delay_cycles(0x100);
    led = blankLed;
    /*
    // Animation - Part1
    // set one LED after an other (one more with each round) with the colors from the LEDs array
    fillFrameBuffer(leds, NUMBEROFLEDS, frameBuffer, ENCODING);
    for(update=1; update <= NUMBEROFLEDS; update++) {
        sendBufferDma(frameBuffer, update);
        __delay_cycles(0xFFF);
    }
    __delay_cycles(0xFFF);
    */
    /*
    // Animation - Part2
    // shift previous LED pattern
    for(update=0; update < 15*8; update++) {
        shiftLed(leds, NUMBEROFLEDS);
        fillFrameBuffer(leds, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
        __delay_cycles(0x7F);
    }

    // Animation - Part3
    led = blankLed;
    // set all LEDs with the same color and simulate a sunrise
    for(colorIdx=0; colorIdx < 0xFF; colorIdx++) {
        led.red = colorIdx + 1;
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
        __delay_cycles(0x1F);
    }
    for(colorIdx=0; colorIdx < 0xD0; colorIdx++) {
        led.green = colorIdx;
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
        __delay_cycles(0x2F);
    }
    */

    for(colorIdx=0; colorIdx < 0x50; colorIdx++) {
        led.blue = 120;//colorIdx;
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
        //__delay_cycles(0x3F);
    }

    //__delay_cycles(0xFFFFF);

}

void rgbLed_blankLed(){
    fillFrameBufferSingleColor(&blankLed, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);

    led = blankLed;
}


void rgbLed_setRed(uint16_t colorVal){

    rgbLed_blankLed();

    for(colorIdx=0; colorIdx < 0xFF; colorIdx++) {
        led.red = colorVal;
        shiftLed(leds, NUMBEROFLEDS);
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
    }

}

void rgbLed_setGreen(uint16_t colorVal){
    rgbLed_blankLed();
    for(colorIdx=0; colorIdx < 0xD0; colorIdx++) {
        led.green = colorVal;
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
    }
}


void rgbLed_setBlue(uint16_t colorVal){
    rgbLed_blankLed();
    //for(colorIdx=0; colorIdx < 0xFF; colorIdx++) {
        led.blue = colorVal;
        fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
        sendBufferDma(frameBuffer, NUMBEROFLEDS);
    //}
}



void rgbLed_setPurple(){
    rgbLed_blankLed();
    led.red = 102;
    led.blue = 153;
    fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);
/*
    led.blue = 153;
    fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);
*/
}


void rgbLed_setSiemensGreen(){
    rgbLed_blankLed();
    led.green = 153;
    led.blue = 153;
    fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);
/*
    led.blue = 153;
    fillFrameBufferSingleColor(&led, NUMBEROFLEDS, frameBuffer, ENCODING);
    sendBufferDma(frameBuffer, NUMBEROFLEDS);
*/
}





void rgbLed_assign(t_colorObj* in, uint16_t run, uint16_t conn, uint16_t charge){
    in->run = run;
    in->conn = conn;
    in->charge = charge;
}


/*
 *
 * RUN CONN CHARGE
 *  1   0     0             (4)
 *  1   0     1             (5)
 *  1   1     0             (6)
 *  1   1     1             (7)
 *
 *  2   0     0             (8)
 *  2   0     1             (9)
 *  2   1     0             (10)
 *  2   1     1             (11)
 *
 */
void rgbLed_drive(t_colorObj* in, t_colorObj* inOld){
    static unsigned int colorSituation = 0;
    colorSituation = in->run<<2 | in->conn<<1 | in->charge;

    if((in->run == inOld->run) && (in->conn == inOld->conn) && (in->charge == inOld->charge)){
        // do nothing
    }
    else{
        switch(colorSituation){
            case 4:
                rgbLed_setBlue(120);
                break;
            case 5:
                rgbLed_setRed(255);
                break;
            case 6:
                rgbLed_setPurple();
                break;
            case 7:
                rgbLed_setBlue(120);
                break;

            case 8:
                rgbLed_setSiemensGreen();
                break;
            case 9:
                rgbLed_setRed(255);
                break;
            case 10:
                rgbLed_setPurple();
                break;
            case 11:
                rgbLed_setSiemensGreen();
                break;
        }
    }

    inOld->run = in->run;
    inOld->conn = in->conn;
    inOld->charge = in->charge;
}



#endif /* RGBLED_H_ */
