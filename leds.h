#ifndef leds_include
#define leds_include

#include <stdbool.h>
#include <stdint.h>

extern bool spi_wakeup;
extern bool spi_transmission_in_progress;

void InitializeLEDInterface(void);
// void UpdateLEDs_MusicMode(int note);
// void UpdateLEDs_ButtonsMode(int input);
bool SendSPIMessage(uint8_t *message_ptr, int message_len);
void RGB_2_SPI(uint8_t *dst, uint8_t r, uint8_t g, uint8_t b);

#endif // leds_include