#include <ti/devices/msp/msp.h>
#include "delay.h"
#include "timing.h"
#include "leds.h"
#include "framebuffer.h"

int main(void)
{
    int cycle  = 0;

    InitializeLEDInterface();
    InitializeTimerG0();

    if (GPIOA->GPRCM.STAT & GPIO_STAT_RESETSTKY_MASK) {
        // Sticky bit is set  so module was reset (or never initialized)
        // Do full reset sequence and clear the sticky bit
        GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                                GPIO_RSTCTL_RESETSTKYCLR_CLR |
                                GPIO_RSTCTL_RESETASSERT_ASSERT);
        GPIOA->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                                GPIO_PWREN_ENABLE_ENABLE);
        delay_cycles(POWER_STARTUP_DELAY);
    }

    IOMUX->SECCFG.PINCM[(IOMUX_PINCM1)] = IOMUX_PINCM_PC_CONNECTED | 1;  // LED GPIO
    GPIOA->DOUT31_0 = 1;
    GPIOA->DOE31_0 = 1;

    // let the buzzer run for 0.1 s just so we know it's there!
    delay_cycles(1600000);

    SetTimerG0Delay(10000); // 20 ticks at 32 kHz is 0.6 ms
    EnableTimerG0();

    // VERY BASIC LOOP - If button 1 signals a 0, enable the PWM
    int row = 2;
    int start_col = -7;
    while (1) {
        if (timer_wakeup) {
            ClearFramebuffer();
            DrawRectangle(7, (start_col++)%16, 3,2, COLOR_DIM_RED);
            DrawRectangle(3, (start_col++)%16, 3,2, COLOR_DIM_GREEN);
//            DrawRectangle(10, (start_col++)%16, 3,2, COLOR_DIM_BLUE);
            FlushFramebuffer();
            timer_wakeup = 0;
        }
        __WFI(); // Go to sleep until timer counts down again.
    }

}
