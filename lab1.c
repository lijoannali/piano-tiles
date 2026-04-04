/*
 * Main Logic Loop for Clock
 */

#include <ti/devices/msp/msp.h>
#include "delay.h"
#include "initialize_leds.h"
#include "state_machine_logic.h"

/* This results in approximately 1s of delay assuming 32 MHz CPU_CLK */
#define DELAY (32000000)

int main(void)
{
    InitializeGPIO();
    
    /* Initialize state machine */
    int currMin = 0; //index to the current minute state
    int currHour = 0; //index to the current hour state
    int output = GPIOA->DOUT31_0;

    while (1) {
    // Use Read, Modify, Write to change time
        output = GPIOA->DOUT31_0;
        /* Change the currently lit active-low LED */
        output |= GetStateOutputGPIOA(minutesFSM, currMin); //Set the current minute to turn it off
        currMin = GetNextState(minutesFSM, currMin);
        output &=  ~(GetStateOutputGPIOA(minutesFSM, currMin)); //Clear the next minute to turn it on
        GPIOA->DOUT31_0 = output;

        if(currMin == 0) //Increment the hour LED after every 12 minute ticks
        {
            output = GPIOA->DOUT31_0;
            output |= (GetStateOutputGPIOA(hoursFSM, currHour)); //turn off current hour LED
            currHour = GetNextState(hoursFSM, currHour);
            output &=  ~(GetStateOutputGPIOA(hoursFSM, currHour)); //turn on next hour LED
            GPIOA->DOUT31_0 = output;
        }
        delay_cycles(DELAY);
    }
}

/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
