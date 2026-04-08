
#include "leds.h"
#include "delay.h"
#include <ti/devices/msp/msp.h>

#define WS2812B_ONE  0b110  // 1 → 110
#define WS2812B_ZERO 0b100  // 0 → 100

// PUBLIC
// Flags for interface to main code
bool spi_wakeup;

// PRIVATE
// Data buffer and length for SPI TX - we'll load this with a function
uint8_t *spi_message;
int      spi_message_len;
int      spi_message_idx;
bool     spi_transmission_in_progress;


void InitializeLEDInterface(void) {
    // 1. Initialize GPIO IOMUX for SPI
    // Check if module needs full initialization
    if (GPIOA->GPRCM.STAT & GPIO_STAT_RESETSTKY_MASK) {
        // Sticky bit is set → module was reset (or never initialized)
        // Do full reset sequence and clear the sticky bit
        GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                                GPIO_RSTCTL_RESETSTKYCLR_CLR |
                                GPIO_RSTCTL_RESETASSERT_ASSERT);
        GPIOA->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                                GPIO_PWREN_ENABLE_ENABLE);
        delay_cycles(POWER_STARTUP_DELAY);
    } 

    if (GPIOB->GPRCM.STAT & GPIO_STAT_RESETSTKY_MASK) {
    GPIOB->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                            GPIO_RSTCTL_RESETSTKYCLR_CLR |
                            GPIO_RSTCTL_RESETASSERT_ASSERT);
    GPIOB->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                            GPIO_PWREN_ENABLE_ENABLE);
    delay_cycles(POWER_STARTUP_DELAY);
    }

    // Initialize SPI0 connections!!
    IOMUX->SECCFG.PINCM[(IOMUX_PINCM22)] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM22_PF_SPI0_SCLK;  // SPI0_SCLK on PA11
    IOMUX->SECCFG.PINCM[(IOMUX_PINCM59)] = IOMUX_PINCM_PC_CONNECTED | 0x1;  // SPI0_SCLK on PA11
    IOMUX->SECCFG.PINCM[(IOMUX_PINCM32)] = IOMUX_PINCM_PC_CONNECTED | 0x1; //PB15 on GPIOB
    //Does GPIOB work?
    IOMUX->SECCFG.PINCM[(IOMUX_PINCM25)] = IOMUX_PINCM_PC_CONNECTED | 0x1; //PB8 on GPIOB

    SPI0->GPRCM.RSTCTL = (SPI_RSTCTL_KEY_UNLOCK_W | SPI_RSTCTL_RESETSTKYCLR_CLR | SPI_RSTCTL_RESETASSERT_ASSERT);
    SPI0->GPRCM.PWREN = (SPI_PWREN_KEY_UNLOCK_W | SPI_PWREN_ENABLE_ENABLE);
    delay_cycles(POWER_STARTUP_DELAY); // delay to enable SPI to turn on and reset

    // Configure clocking for SPI0
    SPI0->CLKSEL = (uint32_t) SPI_CLKSEL_SYSCLK_SEL_ENABLE; // use the SYSOSC
    SPI0->CLKDIV = (uint32_t) SPI_CLKDIV_RATIO_DIV_BY_1; // actually 0x0, which is going to be default, but here for completeness

    // Configure the module
    // SPI0->CTL0 = SPI_CTL0_SPO_HIGH | SPI_CTL0_SPH_SECOND | // Clock edges and phases for data
    //         SPI_CTL0_FRF_MOTOROLA_3WIRE |  // Don't use a chip select pin to bound frames
    //         SPI_CTL0_DSS_DSS_8;
    SPI0->CTL0 = SPI_CTL0_SPO_LOW  |       // CPOL=0: data line idles LOW between frames
             SPI_CTL0_SPH_FIRST |       // CPHA=0: data valid on first clock edge
             SPI_CTL0_FRF_MOTOROLA_3WIRE |
             SPI_CTL0_DSS_DSS_8;

    SPI0->CTL1 = SPI_CTL1_CP_ENABLE | // Microcontroller is CONTROLLER
            SPI_CTL1_PREN_DISABLE | SPI_CTL1_PTEN_DISABLE | // Disable parity on RX and TX
            SPI_CTL1_MSB_ENABLE; // Bit order is MSB first

            

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     2000000 = (32000000)/((1 + 7) * 2)
     */

    SPI0->CLKCTL = 5; // 10 bits

    // LOOK HERE!
    /* Set RX and TX FIFO threshold levels */
    SPI0->IFLS = SPI_IFLS_RXIFLSEL_LEVEL_1 | // Trigger an RX interrupt when FIFO contains >=1 sample (included for reference)
            SPI_IFLS_TXIFLSEL_LVL_EMPTY;     // Trigger an TX interrupt when the FIFO is empty

    /* Enable Transmit FIFO interrupt */
    SPI0->CPU_INT.IMASK |= SPI_CPU_INT_IMASK_TX_SET; // Only enable TX interrupt

    /* Enable module */
    SPI0->CTL1 |= SPI_CTL1_ENABLE_ENABLE;
}

bool SendSPIMessage(uint8_t *message_ptr, int message_len) {
    if (spi_transmission_in_progress ||  message_len == 0) {
        return false;
    }
    else {
        spi_message = message_ptr;
        spi_message_len = message_len;
        spi_transmission_in_progress = true;
        spi_message_idx = 1; // We'll load the first item in a moment, so the ISR should start at the second
        NVIC_ClearPendingIRQ(SPI0_INT_IRQn);
        NVIC_EnableIRQ(SPI0_INT_IRQn);
        SPI0->TXDATA = spi_message[0]; // This actually initiates transmission
        return true;
    }
}

void SPI0_IRQHandler(void)
{
    switch (SPI0->CPU_INT.IIDX) {
        case SPI_CPU_INT_IIDX_STAT_TX_EVT: // SPI interrupt index for transmit FIFO
            SPI0->TXDATA = spi_message[spi_message_idx];
            spi_message_idx++;
            spi_wakeup = 1;
            if (spi_message_idx == spi_message_len) { // We're done!
               spi_transmission_in_progress = false; 
               NVIC_DisableIRQ(SPI0_INT_IRQn);
            }
            break;
        default:
            break;
    }
}


//Convert RGB colors to representation in WS2812B bits
/* Input: 3 r, g, b color values 
   Output: 96 WS2812B bits in grb and LSB to MSB order
*/
// void RGB_2_SPI(uint16_t *dst, uint8_t r, uint8_t g, uint8_t b) {
//     uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b; //Array uses GRB order
//     for (int i = 0; i < 6; i++) dst[i] = 0;  // clear buffer
//     for (int i = 0; i < 24; i++) {
//         uint8_t  bit     = (grb >> (23 - i)) & 1; //bits read L to R
//         int      word_idx = i / 4; //Go to start of the current 4-bit word 
//         int      bit_pos  = 3 - (i % 4);//MSB that come first ...
//         uint16_t pattern  = bit ? WS2812B_ONE : WS2812B_ZERO; //Convert 1 bit --> 4 WS2812B bits
//         dst[word_idx] |= (pattern << (bit_pos * 4)); //... are shifted MORE to the left
//     }
// }


void RGB_2_SPI(uint8_t *dst, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

    for (int i = 0; i < 9; i++) dst[i] = 0;

    for (int i = 0; i < 24; i++) {
        uint8_t bit     = (grb >> (23 - i)) & 1;
        uint8_t pattern = bit ? WS2812B_ONE : WS2812B_ZERO;

        int spi_bit_start = i * 3;
        // removed unused bit_offset variable

        for (int p = 0; p < 3; p++) {
            uint8_t pbit   = (pattern >> (2 - p)) & 1;
            int     abs_bit = spi_bit_start + p;
            int     b_idx   = abs_bit / 8;
            int     b_off   = 7 - (abs_bit % 8);
            dst[b_idx] |= (pbit << b_off);
        }
    }
}

void get_LED_bits(uint8_t *dst, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    r = ((uint8_t)r * brightness) / 255;
    g = ((uint8_t)g * brightness) / 255;
    b = ((uint8_t)b * brightness) / 255;
    RGB_2_SPI(dst, r, g, b);
}


// dst must point to a buffer of at least 9 bytes per LED (24 bits * 3 = 72 bits = 9 bytes)
// void RGB_2_SPI(uint8_t *dst, uint8_t r, uint8_t g, uint8_t b) {
//     uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

//     // Clear 9 bytes
//     for (int i = 0; i < 9; i++) dst[i] = 0;
//     for (int i = 0; i < 24; i++) {
//         uint8_t bit      = (grb >> (23 - i)) & 1;
//         uint8_t pattern  = bit ? WS2812B_ONE : WS2812B_ZERO;  // 3-bit pattern

//         int spi_bit_start = i * 3;           // which SPI bit position (0..71)
//         int byte_idx      = spi_bit_start / 8;
//         int bit_offset    = 7 - (spi_bit_start % 8);  // MSB first

//         // Pattern is 3 bits wide, may span 2 bytes
//         for (int p = 0; p < 3; p++) {
//             uint8_t pbit = (pattern >> (2 - p)) & 1;  // MSB of pattern first
//             int abs_bit  = spi_bit_start + p;
//             int b_idx    = abs_bit / 8;
//             int b_off    = 7 - (abs_bit % 8);
//             dst[b_idx]  |= (pbit << b_off);
//         }
//     }
// }

//Get the bit level representation of a R, G, B color given the brightness and colors
/* Input: 0 to 255 brightness from Dark to Light 
 * Output: 96 WS2812B bits in grb and LSB to MSB order, scaled by brightness 
 */
// void get_LED_bits(uint8_t *dst, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
//     // brightness 0-255, scale each channel down
//     r = ((uint16_t)r * brightness) / 255;
//     g = ((uint16_t)g * brightness) / 255;
//     b = ((uint16_t)b * brightness) / 255;
//     RGB_2_SPI(dst, r, g, b);
// }