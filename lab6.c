#include <ti/devices/msp/msp.h>
#include "delay.h"
#include "timing.h"
#include "leds.h"
#include "framebuffer.h"
#include "game.h"

#define SW1 ((uint32_t) 0x1 << 24)  // PA24
#define SW2 ((uint32_t) 0x1 << 25)  // PA25
#define SW3 ((uint32_t) 0x1 << 26)  // PA26
#define SW4 ((uint32_t) 0x1 << 27)  // PA27

void InitButtonGPIO(void) {
    if (GPIOA->GPRCM.STAT & GPIO_STAT_RESETSTKY_MASK) {
        GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                                GPIO_RSTCTL_RESETSTKYCLR_CLR |
                                GPIO_RSTCTL_RESETASSERT_ASSERT);
        GPIOA->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                                GPIO_PWREN_ENABLE_ENABLE);
        delay_cycles(POWER_STARTUP_DELAY);
    }
    const uint32_t cfg = IOMUX_PINCM_PC_CONNECTED |
                         IOMUX_PINCM_INENA_ENABLE  |
                         ((uint32_t) 0x00000001)   |
                         IOMUX_PINCM_PIPU_ENABLE   |
                         IOMUX_PINCM_PIPD_DISABLE;

    IOMUX->SECCFG.PINCM[IOMUX_PINCM54] = cfg;  // PA24
    IOMUX->SECCFG.PINCM[IOMUX_PINCM55] = cfg;  // PA25
    IOMUX->SECCFG.PINCM[IOMUX_PINCM59] = cfg;  // PA26
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = cfg;  // PA27
    delay_cycles(POWER_STARTUP_DELAY);
}

int main(void) {
    InitializeLEDInterface();
    InitializeTimerG0();
    InitButtonGPIO();

    delay_cycles(1600000);

    SetTimerG0Delay(983);  // LFCLK=32768Hz → ~30ms per tick
    EnableTimerG0();

    game_state_t state = InitGame();

    while (1) {
        if (timer_wakeup) {
            timer_wakeup = false;
            uint32_t input = GPIOA->DIN31_0 & (SW1 | SW2 | SW3 | SW4);
            state = UpdateGame(state, input);
            RenderGame(state);
        }
        __WFI();
    }
}