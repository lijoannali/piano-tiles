#include <ti/devices/msp/msp.h>
// #include</Users/joanna/ti/mspm0_sdk_2_09_00_01/source/ti/devices/msp/m0p/mspm0g350x.h>
#include "delay.h"
#include "buttons.h"
#include "timing.h"
#include "buzzer.h"
#include "leds.h"
// #include "leds_poll.h"

// #define NUM_LEDS      256
// #define BYTES_PER_LED 9

// uint8_t buf[NUM_LEDS * BYTES_PER_LED];  // 2304 bytes, global = safe

// uint8_t rgb_buf[NUM_LEDS * 3];  // ← was uint8_t buf[NUM_LEDS * 9]

// int main(void) {
//     InitializeLEDInterface_Poll();          // ← was InitializeLEDInterface()

//     FillStrip_Poll(rgb_buf, NUM_LEDS, 50, 0, 0);  // ← replaces the for loop + SendSPIMessage

//     while(1);
// }
// int main(void) {
//     InitializeLEDInterface();

//     for (int i = 0; i < NUM_LEDS; i++) {
//         RGB_2_SPI(&buf[i * BYTES_PER_LED], 50, 0, 0);  // RED
//     }

//     SendSPIMessage(buf, NUM_LEDS * BYTES_PER_LED);

//     while(1);
// }

uint8_t test_packet[] = {0xAA, 0x55, 0xFF, 0x00};
uint8_t message_len = 4;

//Test SPI message!

// red = 110 110 110 110 110 110 110 110
// = 11011011 01101101 10110110

// this array has 6 red RGB codes, each red code is the sequence: red1,red2,red3
// uint8_t red[] = {red1, red2, red3, red1, red2, red3, red1, red2, red3, red1, red2, red3, red1, red2, red3, red1, red2, red3};
// uint8_t red_length = sizeof(red)/sizeof(red[0]); 
#define red1 0b11011011
#define red2 0b01101101
#define red3 0b10110110
#define zero1 0b10010010
#define zero2 0b01001001
#define zero3 0b00100100
#define reset 0b00000000

// red = 110 110 110 110 110 110 110 110
// = 11011011 01101101 10110110

// this array has 6 red RGB codes, each red code is the sequence: zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3
uint8_t red[] = {
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3,
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3,
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3,
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3,
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3,
    zero1, zero2, zero3, red1, red2, red3, zero1, zero2, zero3
};
uint8_t red_length = sizeof(red)/sizeof(red[0]); 


int main(void) {
    InitializeLEDInterface();
    delay_cycles(16000);
    GPIOA->DOESET31_0 = (1 << 26);
    GPIOA->DOUT31_0 |= (1 << 26);

    //We want to use the SPI on this pin!
    GPIOB->DOESET31_0 = (1 << 8);
    GPIOB->DOUT31_0 |= (1 << 8);

    GPIOB->DOESET31_0 = (1 << 15);
    GPIOB->DOUT31_0 |= (1 << 15);

    while (1) {
        while (!SendSPIMessage(red, 27)) {
            // wait for previous to finish
        }
        delay_cycles(1000000); // gap between bursts
    }



    // while (!SendSPIMessage(onTxPacket, message_len)) 
    // {
    //     // Block until previous message is complete                
    // }
    //Test that your PA9 is working
    // GPIOA->DOESET31_0 = (1 << 9);
    // GPIOA->DOUT31_0 |= (1 << 9);
    // while(1);
}


