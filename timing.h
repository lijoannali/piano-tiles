#ifndef timing_include
#define timing_include
#include <stdint.h>
#include <stdbool.h>

#define POWER_STARTUP_DELAY 16
#define DEBOUNCE_TICKS 5

extern bool timer_wakeup;

void InitializeTimerG0(void);
void SetTimerG0Delay(uint16_t delay);
void EnableTimerG0(void);




#endif /* timing_include */
