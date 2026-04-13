#include <ti/devices/msp/msp.h>
#include "blink_helper.h"

#define POWER_STARTUP_DELAY 16
#define SW1 ((uint32_t) 0x1 << 26)  // PA26/CM59

volatile int up_count        = 0;
volatile int down_count      = 0;
volatile int currently_pressed = 0;

int main(void) {
    GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                            GPIO_RSTCTL_RESETSTKYCLR_CLR |
                            GPIO_RSTCTL_RESETASSERT_ASSERT);
    GPIOA->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                            GPIO_PWREN_ENABLE_ENABLE);
    delay_cycles(POWER_STARTUP_DELAY);

    // PA26 as input with pull-up
    IOMUX->SECCFG.PINCM[IOMUX_PINCM59] =
        IOMUX_PINCM_PC_CONNECTED |
        IOMUX_PINCM_INENA_ENABLE |
        ((uint32_t) 0x00000001) |
        IOMUX_PINCM_PIPU_ENABLE |
        IOMUX_PINCM_PIPD_DISABLE;

    delay_cycles(POWER_STARTUP_DELAY);

    int prev_pressed = 0;

    while (1) {
        uint32_t input = GPIOA->DIN31_0 & SW1;
        int pressed = (input == 0);  // active low - 0 means button pressed

        if (pressed && !prev_pressed) {
            down_count++;
            currently_pressed = 1;
        }
        else if (!pressed && prev_pressed) {
            up_count++;
            currently_pressed = 0;
        }

        prev_pressed = pressed;
        delay_cycles(160000);
    }
}