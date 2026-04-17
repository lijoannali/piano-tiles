#include <ti/devices/msp/msp.h>
#include "delay.h"
#include "timing.h"
#include "leds.h"
#include "framebuffer.h"

int main(void){
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

    // let the buzzer run for 0.1 s
    delay_cycles(1600000);

    SetTimerG0Delay(10000); // 20 ticks at 32 kHz is 0.6 ms
    EnableTimerG0();

    // Falling Rectangles example
    //Start them at offsetted heights
    int start_col_red = 0;
    int start_col_blue = 1;
    int start_col_green = 2;
    int start_col_green2 = 3;

    while (1) {
        if (timer_wakeup) {
            //Clear leftover bits from the prev frame at the start of every new frame
            ClearFramebuffer();

            //Rectangles are drawn in top to bottom order (order of function calls), so the later ones would overlap
            //These functions edit the rectangles and put them into the framebuffer
            //There is no SPI call at this point
            //Function signature: x,y = top left corner of rectangle, h,w = size
            DrawRectangle(1, (start_col_red++)%16, 3,2, COLOR_DIM_RED);
            DrawRectangle(5, (start_col_blue++)%16, 3,2, COLOR_DIM_BLUE);
            DrawRectangle(9, (start_col_green++)%16, 3,2, COLOR_DIM_GREEN);
            DrawRectangle(13, (start_col_green2++)%16, 7,2, COLOR_DIM_GREEN);

            //This function is what converts the framebuffer into the SPI message, and sends the SPI message out
            FlushFramebuffer(); //Draws frame buffer to display
            timer_wakeup = 0;
        }
        __WFI(); // Sleep until timer counts down again.
    }

}
