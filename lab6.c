#include <ti/devices/msp/msp.h>
// #include</Users/joanna/ti/mspm0_sdk_2_09_00_01/source/ti/devices/msp/m0p/mspm0g350x.h>
#include "delay.h"
#include "buttons.h"
#include "timing.h"
#include "buzzer.h"
#include "leds.h"
// #include "leds_poll.h"

#define NUM_LEDS      256
#define BYTES_PER_LED 9

uint8_t buf[NUM_LEDS * BYTES_PER_LED];  // 2304 bytes, global = safe

uint8_t rgb_buf[NUM_LEDS * 3];  // ← was uint8_t buf[NUM_LEDS * 9]

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
    while(1);
}


