#include "ti/devices/msp/msp.h"

void SPI0_init(void) {
    // Enable clocks for SPI0 and GPIOA
    SYSCTRL->SOCLOCK.PCLKCFG0 |= SYSCTL_PCLKCFG0_SPI0_MASK;
    SYSCTRL->SOCLOCK.PCLKCFG0 |= SYSCTL_PCLKCFG0_GPIOA_MASK;

    // Configure PA10 as SPI0 MOSI (alternate function)
    GPIOA->GPRCM.IOMUX->PINCM[10] = 
        IOMUX_PINCM_PC_MASK |        // connected
        (0x2 << IOMUX_PINCM_PF_OFS); // alt function 2 = SPI0 MOSI

    // Reset SPI0
    SPI0->GPRCM.RSTCTL = 
        SPI_RSTCTL_KEY_UNLOCK_W |
        SPI_RSTCTL_RESETSTKYCLR_CLR |
        SPI_RSTCTL_RESETASSERT_ASSERT;

    // Enable SPI0 power
    SPI0->GPRCM.PWREN = 
        SPI_PWREN_KEY_UNLOCK_W |
        SPI_PWREN_ENABLE_ENABLE;

    // Set clock divider for ~3.2 MHz (32 MHz / 10)
    SPI0->CLKDIV2 = 0;   // no extra divider
    SPI0->CLKCTL  = 9;   // divide by 10

    // Configure: Motorola 8-bit, CPOL=0, CPHA=0, controller mode
    SPI0->CTL0 = 
        SPI_CTL0_SPO_LOW   |   // CPOL=0
        SPI_CTL0_SPH_FIRST |   // CPHA=0
        SPI_CTL0_DSS_8     |   // 8-bit
        SPI_CTL0_FRF_MOTO;     // Motorola frame format

    SPI0->CTL1 = 
        SPI_CTL1_MSB_ENABLE |  // MSB first
        SPI_CTL1_ENABLE_ENABLE;// enable SPI
}

void SPI0_send_byte(uint8_t data) {
    while (SPI0->STAT & SPI_STAT_TNF_MASK == 0); // wait for TX FIFO space
    SPI0->TXDATA = data;
}

void SPI0_wait_idle(void) {
    while (SPI0->STAT & SPI_STAT_BUSY_MASK); // wait until not busy
}