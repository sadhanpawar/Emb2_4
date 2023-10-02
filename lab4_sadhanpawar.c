/*
 * lab4_sadhanpawar.c
 *
 *  Created on: Mar 6, 2023
 *      Author: sadhan
 */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "nvic.h"
#include "wait.h"

// Pins
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3
#define BLUE_LED PORTF,2

#define PUSH_BUTTON_1 PORTF,4
#define PUSH_BUTTON_2 PORTF,0

static uint8_t wakeupSrc = 0;

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTF);

    SYSCTL_SCGCGPIO_R |= SYSCTL_SCGCGPIO_S5;

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(BLUE_LED);
    selectPinDigitalInput(PUSH_BUTTON_1);
    enablePinPullup(PUSH_BUTTON_1);
    selectPinDigitalInput(PUSH_BUTTON_2);
    enablePinPullup(PUSH_BUTTON_2);

    /*enable oscillator*/
    while(!(HIB_CTL_R & HIB_CTL_WRC)); /*HIB_CTL*/
    HIB_CTL_R = 0x40;

    /*60secs in future*/
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_RTCM0_R = 0x3C; /* 60sec ? */

    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_RTCSS_R = 0x0; /*HIB_RTCSS_R*/

    /*rtc load value*/
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_RTCLD_R = 0x0; /*load ref value*/

    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R |= 0x158;

#if 0
    /*clear interrupts*/
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_IC_R = 0x1F;
#endif

    /*enable rtc and ext interrupt*/
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_IM_R = 0x9;

    enableNvicInterrupt(INT_HIBERNATE);
}

/* Table:
 * normal mode : 18.39 ma(milli amps)
 * hibernation mode : 6.39 ua(micro amps)
 * after reset : 20 ma surges and comes back to 18.39ma
 * */

int main(void)
{
    // Initialize hardware
    initHw();

    if(wakeupSrc == 1) {
        setPinValue(RED_LED, 1);
        waitMicrosecond(1000000);
        setPinValue(RED_LED, 0);
        wakeupSrc = 0;
    } else if (wakeupSrc == 2) {
        setPinValue(BLUE_LED, 1);
        waitMicrosecond(1000000);
        setPinValue(BLUE_LED, 0);
        wakeupSrc = 0;
    } else {
        /*do nothing*/
    }

    /*wait for sw1 press*/
    setPinValue(GREEN_LED, 1);
    while(getPinValue(PUSH_BUTTON_1));

    /*clear all the leds */
    setPinValue(RED_LED, 0);
    setPinValue(GREEN_LED, 0);
    setPinValue(BLUE_LED, 0);

    /*enable rtc*/
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R |= 0x1;

    /*request hibernation */
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R |= 0x2;

    while(true);

}

void IntHandlerHibernate(void)
{
    if(HIB_RIS_R & HIB_RIS_RTCALT0) {
        wakeupSrc = 1;
    } else if (HIB_RIS_R & HIB_RIS_EXTW) {
        wakeupSrc = 2;
    }

    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_IC_R = 0x1F;
}

