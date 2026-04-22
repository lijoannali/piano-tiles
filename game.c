#include "game.h"
#include "framebuffer.h"
#include "delay.h"
#include <ti/devices/msp/msp.h>

#define SW1 ((uint32_t) 0x1 << 24)  // PA24
#define SW2 ((uint32_t) 0x1 << 25)  // PA25
#define SW3 ((uint32_t) 0x1 << 26)  // PA26
#define SW4 ((uint32_t) 0x1 << 27)  // PA27

// 16 cols split into 4 lanes of 4 cols each.
// Tile is 2 cols wide, centered in its lane.
// Lane 0: cols 0-3,  tile at col 1
// Lane 1: cols 4-7,  tile at col 5
// Lane 2: cols 8-11, tile at col 9
// Lane 3: cols 12-15,tile at col 13
static const int TILE_COL_START[4] = {1, 5, 9, 13};
static const int TILE_COL_WIDTH    = 2;
static const int TILE_ROW_HEIGHT   = 4;

static const uint8_t TILE_COLOR[4] = {
    COLOR_DIM_RED,
    COLOR_DIM_GREEN,
    COLOR_DIM_BLUE,
    COLOR_DIM_RED
};

#define HIT_FLASH_TICKS  6

static uint32_t rng_state = 12345;
static uint32_t NextRand(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

game_state_t InitGame(void) {
    game_state_t s;
    s.mode  = MODE_STARTUP_ANIM;
    s.score = 0;
    s.misses = 0;
    s.tick   = 0;
    s.anim_counter = 0;
    s.next_col = 0;
    s.spawn_counter = 0;
    for (int i = 0; i < NUM_LANES; i++) {
        s.tiles[i].active = false;
        s.tiles[i].state  = TILE_GONE;
        s.tiles[i].row    = 0;
        s.buttons[i].state = BTN_IDLE;
        s.buttons[i].debounce_counter = 0;
        s.buttons[i].just_became_held = false;
        s.fall_counter[i] = 0;
        s.hit_flash[i]    = 0;
    }
    return s;
}

static btn_t UpdateButton(btn_t btn, bool raw_pressed) {
    btn_t b = btn;
    b.just_became_held = false;
    switch (btn.state) {
        case BTN_IDLE:
            if (raw_pressed) { b.state = BTN_DEBOUNCING; b.debounce_counter = 1; }
            break;
        case BTN_DEBOUNCING:
            if (raw_pressed) {
                b.debounce_counter++;
                if (b.debounce_counter >= DEBOUNCE_LIMIT) { b.state = BTN_HELD; b.just_became_held = true; }
            } else { b.state = BTN_IDLE; b.debounce_counter = 0; }
            break;
        case BTN_HELD:
            if (!raw_pressed) { b.state = BTN_IDLE; b.debounce_counter = 0; }
            break;
    }
    return b;
}

static tile_t SpawnTile(void) {
    tile_t t;
    t.active = true;
    t.row    = -TILE_ROW_HEIGHT;
    t.state  = TILE_FALLING;
    return t;
}

game_state_t UpdateGame(game_state_t s, uint32_t gpio_input) {
    s.tick++;
    rng_state ^= s.tick;

    bool raw_btn[NUM_LANES];
    raw_btn[0] = ((gpio_input & SW1) == 0);
    raw_btn[1] = ((gpio_input & SW2) == 0);
    raw_btn[2] = ((gpio_input & SW3) == 0);
    raw_btn[3] = ((gpio_input & SW4) == 0);

    for (int i = 0; i < NUM_LANES; i++)
        s.buttons[i] = UpdateButton(s.buttons[i], raw_btn[i]);

    bool any_btn_just_held = false;
    for (int i = 0; i < NUM_LANES; i++)
        if (s.buttons[i].just_became_held) any_btn_just_held = true;

    for (int i = 0; i < NUM_LANES; i++)
        if (s.hit_flash[i] > 0) s.hit_flash[i]--;

    switch (s.mode) {

        case MODE_STARTUP_ANIM:
            s.anim_counter++;
            if (any_btn_just_held) {
                s.mode = MODE_PLAYING;
                s.score = 0; s.misses = 0; s.spawn_counter = 0;
                for (int i = 0; i < NUM_LANES; i++) {
                    s.tiles[i].active = false;
                    s.buttons[i].state = BTN_IDLE;
                    s.buttons[i].debounce_counter = 0;
                    s.fall_counter[i] = 0;
                    s.hit_flash[i] = 0;
                }
                s.next_col = NextRand() % NUM_LANES;
                s.tiles[s.next_col] = SpawnTile();
                s.fall_counter[s.next_col] = 0;
            }
            break;

        case MODE_PLAYING: {
            bool any_active = false;
            for (int i = 0; i < NUM_LANES; i++)
                if (s.tiles[i].active) any_active = true;

            if (!any_active) {
                s.spawn_counter++;
                if (s.spawn_counter >= SPAWN_GAP) {
                    s.next_col = NextRand() % NUM_LANES;
                    s.tiles[s.next_col] = SpawnTile();
                    s.fall_counter[s.next_col] = 0;
                    s.spawn_counter = 0;
                }
            }

            for (int i = 0; i < NUM_LANES; i++) {
                if (!s.tiles[i].active) continue;

                s.fall_counter[i]++;
                if (s.fall_counter[i] >= FALL_SPEED) {
                    s.fall_counter[i] = 0;
                    s.tiles[i].row++;
                }

                if (s.buttons[i].just_became_held) {
                    s.hit_flash[i] = HIT_FLASH_TICKS;
                    s.tiles[i].active = false;
                    s.tiles[i].state  = TILE_GONE;
                    s.score++;
                    s.spawn_counter = 0;
                    continue;
                }

                if (s.tiles[i].row >= MATRIX_ROWS) {
                    s.misses++;
                    s.tiles[i].active = false;
                    s.tiles[i].state  = TILE_GONE;
                    s.spawn_counter = 0;
                }
            }

            if (s.misses >= MISS_LIMIT) {
                s.mode = MODE_LOSE_ANIM;
                s.anim_counter = 0;
            }
            break;
        }

        case MODE_WIN_ANIM:
        case MODE_LOSE_ANIM:
            s.anim_counter++;
            if (any_btn_just_held) { s = InitGame(); s.mode = MODE_STARTUP_ANIM; }
            break;
    }

    return s;
}

void RenderGame(game_state_t s) {
    ClearFramebuffer();

    switch (s.mode) {

        case MODE_STARTUP_ANIM: {
            int row = (s.anim_counter / 4) % MATRIX_ROWS;
            DrawRow(row, COLOR_DIM_BLUE);
            break;
        }

        case MODE_PLAYING: {
            for (int i = 0; i < NUM_LANES; i++) {
                if (s.hit_flash[i] > 0) {
                    DrawRectangle(MATRIX_ROWS - TILE_ROW_HEIGHT,
                                  TILE_COL_START[i], TILE_COL_WIDTH,
                                  TILE_ROW_HEIGHT, COLOR_DIM_GREEN);
                }
            }
            for (int i = 0; i < NUM_LANES; i++) {
                if (!s.tiles[i].active) continue;
                int top = s.tiles[i].row;
                int bot = top + TILE_ROW_HEIGHT;
                if (bot <= 0 || top >= MATRIX_ROWS) continue;
                int draw_top = (top < 0) ? 0 : top;
                int draw_h   = (bot > MATRIX_ROWS ? MATRIX_ROWS : bot) - draw_top;
                DrawRectangle(draw_top, TILE_COL_START[i], TILE_COL_WIDTH, draw_h, TILE_COLOR[i]);
            }
            break;
        }

        case MODE_WIN_ANIM:
            if ((s.anim_counter / 8) % 2 == 0)
                for (int r = 0; r < MATRIX_ROWS; r++) DrawRow(r, COLOR_DIM_GREEN);
            break;

        case MODE_LOSE_ANIM:
            if ((s.anim_counter / 8) % 2 == 0)
                for (int r = 0; r < MATRIX_ROWS; r++) DrawRow(r, COLOR_DIM_RED);
            break;
    }

    FlushFramebuffer();
}