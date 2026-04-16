#include "framebuffer.h"
#include "leds.h"
#include "string.h"

#define GET_LAST_TWO_BITS 0x03
#define WS2812B_RESET 0x00
#define NUM_LED_COLS 16
#define NUM_LED_ROWS 16
#define NUM_LEDS  (NUM_LED_COLS * NUM_LED_ROWS)
#define NUM_BYTES_PER_ROW (NUM_LED_COLS * BUFFER_BITS_PER_LED) / 8
#define BUFFER_BITS_PER_LED 2
#define BYTES_PER_LED_SPI       9
#define NUM_SPI_RESET_BYTES     2
#define TX_BUFFER_SIZE          (NUM_LEDS * BYTES_PER_LED_SPI + NUM_SPI_RESET_BYTES)
#define FRAMEBUFFER_BYTES       (NUM_LEDS * BUFFER_BITS_PER_LED / 8)  // 2 bits per LED

// 256 pixels x 2 bits--> 512 bits = 64 bytes
static uint8_t framebuffer[FRAMEBUFFER_BYTES];
static uint8_t tx_buffer[TX_BUFFER_SIZE];
// 9 SPI bytes per WS2812B bit pattern: GRB order
//Defined color sequences for a single LED:
static const uint8_t SPI_OFF[]     = {0x92,0x49,0x24,0x92,0x49,0x24,0x92,0x49,0x24}; // G, R, B = 0
static const uint8_t SPI_DIM_RED[] = {0x92,0x49,0x24,0x92,0x4D,0x34,0x92,0x49,0x24}; // R=dim, G=0, B=0
static const uint8_t SPI_DIM_GREEN[] = {0x92,0x4D,0x34,0x92,0x49,0x24,0x92,0x49,0x24}; // G=dim, R=0, B=0
static const uint8_t SPI_DIM_BLUE[]  = {0x92,0x49,0x24,0x92,0x49,0x24,0x92,0x4D,0x34}; // G=0, R=0, B=dim

//Sends out SPI message
void FlushFramebuffer(void) {
    int tx_idx = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        int bit_idx = i * BUFFER_BITS_PER_LED;
        int byte_idx = bit_idx / 8;
        int shift_to_lsb = bit_idx % 8;
        uint8_t color = (framebuffer[byte_idx] >> (shift_to_lsb)) & GET_LAST_TWO_BITS;
        //Write the 9-byte SPI bytes for LED's color to tx_buffer
        const uint8_t *led_color_SPI_bytes;
        switch(color) {
            case COLOR_DIM_RED:   led_color_SPI_bytes = SPI_DIM_RED;   break;
            case COLOR_DIM_GREEN: led_color_SPI_bytes = SPI_DIM_GREEN; break;
            case COLOR_DIM_BLUE:  led_color_SPI_bytes = SPI_DIM_BLUE;  break;
            default:              led_color_SPI_bytes = SPI_OFF;        break;
        }
        for (int j = 0; j < 9; j++) tx_buffer[tx_idx++] = led_color_SPI_bytes[j];
    }
    //Include reset message
    tx_buffer[TX_BUFFER_SIZE - 2] = WS2812B_RESET;
    tx_buffer[TX_BUFFER_SIZE - 1] = WS2812B_RESET;
    SendSPIMessage(tx_buffer, TX_BUFFER_SIZE);
}

void ClearFramebuffer(void) {
    for (int i = 0; i < 64; i++) framebuffer[i] = 0;
}
void DrawRow(int row, uint8_t color) {
    for (int col = 0; col < 16; col++) {
        int bit_idx = (row * 16 + col) * 2;
        int byte_idx = bit_idx / 8;
        int shift = bit_idx % 8;
        framebuffer[byte_idx] &= ~(0x03 << shift);
        framebuffer[byte_idx] |=  (color & 0x03) << shift;
    }
}
