#ifndef game_include
#define game_include

#include <stdint.h>
#include <stdbool.h>

#define MATRIX_ROWS     16
#define MATRIX_COLS     16
#define NUM_LANES        4
#define MISS_LIMIT       1
#define FALL_SPEED       8
#define SPAWN_GAP       30
#define DEBOUNCE_LIMIT   1

typedef enum { TILE_FALLING, TILE_AT_BOTTOM, TILE_GONE } tile_state_t;

typedef struct {
    tile_state_t state;
    int          row;
    bool         active;
} tile_t;

typedef enum { BTN_IDLE, BTN_DEBOUNCING, BTN_HELD } btn_state_t;

typedef struct {
    btn_state_t state;
    int         debounce_counter;
    bool        just_became_held;
} btn_t;

typedef enum { MODE_STARTUP_ANIM, MODE_PLAYING, MODE_WIN_ANIM, MODE_LOSE_ANIM } game_mode_t;

typedef struct {
    game_mode_t  mode;
    tile_t       tiles[NUM_LANES];
    btn_t        buttons[NUM_LANES];
    int          score;
    int          misses;
    uint32_t     tick;
    uint32_t     fall_counter[NUM_LANES];
    uint32_t     spawn_counter;
    int          next_col;
    int          anim_counter;
    int          hit_flash[NUM_LANES];
} game_state_t;

game_state_t InitGame(void);
game_state_t UpdateGame(game_state_t state, uint32_t gpio_input);
void         RenderGame(game_state_t state);

#endif