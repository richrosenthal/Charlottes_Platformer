#include <gb/gb.h>
#include <stdint.h>

#define MAP_WIDTH 20u
#define MAP_HEIGHT 18u
#define TILE_SIZE 8u

#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 16
#define PLAYER_START_X 16
#define PLAYER_START_Y 104

#define FP_SHIFT 4
#define WALK_ACCEL 2
#define WALK_DRAG 1
#define WALK_SPEED 24
#define GRAVITY 2
#define MAX_FALL_SPEED 48
#define JUMP_SPEED -48

static const uint8_t background_tiles[] = {
    /* Empty tile */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* Solid stone tile */
    0xFF, 0xFF, 0x81, 0xBD, 0xBD, 0xA5, 0xA5, 0xBD,
    0xBD, 0x81, 0xFF, 0xFF, 0x81, 0xBD, 0xBD, 0xA5
};

static const uint8_t player_tiles[] = {
    /* Charlotte's head */
    0x3C, 0x3C, 0x7E, 0x42, 0xDB, 0x81, 0xFF, 0xA5,
    0xFF, 0xBD, 0x7E, 0x42, 0x3C, 0x3C, 0x18, 0x18,
    /* Charlotte's body */
    0x3C, 0x3C, 0x7E, 0x5A, 0xFF, 0xA5, 0xDB, 0xBD,
    0x7E, 0x66, 0x66, 0x66, 0x66, 0x42, 0xC3, 0xC3
};

static uint8_t level_map[MAP_WIDTH * MAP_HEIGHT];
static int16_t player_x;
static int16_t player_y;
static int8_t velocity_x;
static int8_t velocity_y;
static uint8_t on_ground;

static void fill_platform(uint8_t x, uint8_t y, uint8_t width) {
    uint8_t i;

    for (i = 0u; i != width; ++i) {
        level_map[(uint16_t)y * MAP_WIDTH + x + i] = 1u;
    }
}

static void build_level(void) {
    uint16_t i;
    uint8_t y;

    for (i = 0u; i != (MAP_WIDTH * MAP_HEIGHT); ++i) {
        level_map[i] = 0u;
    }

    for (y = 0u; y != MAP_HEIGHT; ++y) {
        level_map[(uint16_t)y * MAP_WIDTH] = 1u;
        level_map[(uint16_t)y * MAP_WIDTH + (MAP_WIDTH - 1u)] = 1u;
    }

    fill_platform(0u, 17u, MAP_WIDTH);
    fill_platform(2u, 15u, 6u);
    fill_platform(10u, 12u, 6u);
    fill_platform(4u, 9u, 5u);
    fill_platform(12u, 6u, 6u);
}

static uint8_t is_solid(int16_t pixel_x, int16_t pixel_y) {
    uint8_t tile_x;
    uint8_t tile_y;

    if ((pixel_x < 0) || (pixel_y < 0) ||
        (pixel_x >= (int16_t)(MAP_WIDTH * TILE_SIZE)) ||
        (pixel_y >= (int16_t)(MAP_HEIGHT * TILE_SIZE))) {
        return 1u;
    }

    tile_x = (uint8_t)pixel_x >> 3;
    tile_y = (uint8_t)pixel_y >> 3;
    return level_map[(uint16_t)tile_y * MAP_WIDTH + tile_x] != 0u;
}

static void update_horizontal(void) {
    int16_t next_player_x = player_x + velocity_x;
    int16_t next_x = next_player_x >> FP_SHIFT;
    int16_t top = (player_y >> FP_SHIFT) + 1;
    int16_t bottom = (player_y >> FP_SHIFT) + PLAYER_HEIGHT - 1;

    if (velocity_x > 0) {
        int16_t right = next_x + PLAYER_WIDTH - 1;
        if (is_solid(right, top) || is_solid(right, bottom)) {
            next_x = ((right >> 3) << 3) - PLAYER_WIDTH;
            next_player_x = next_x << FP_SHIFT;
            velocity_x = 0;
        }
    } else if (velocity_x < 0) {
        if (is_solid(next_x, top) || is_solid(next_x, bottom)) {
            next_x = ((next_x >> 3) + 1) << 3;
            next_player_x = next_x << FP_SHIFT;
            velocity_x = 0;
        }
    }

    player_x = next_player_x;
}

static void update_vertical(void) {
    int16_t next_player_y = player_y + velocity_y;
    int16_t next_y = next_player_y >> FP_SHIFT;
    int16_t left = (player_x >> FP_SHIFT) + 1;
    int16_t right = (player_x >> FP_SHIFT) + PLAYER_WIDTH - 2;

    on_ground = 0u;

    if (velocity_y > 0) {
        int16_t bottom = next_y + PLAYER_HEIGHT - 1;
        if (is_solid(left, bottom) || is_solid(right, bottom)) {
            next_y = ((bottom >> 3) << 3) - PLAYER_HEIGHT;
            next_player_y = next_y << FP_SHIFT;
            velocity_y = 0;
            on_ground = 1u;
        } else if (is_solid(left, next_y + PLAYER_HEIGHT) ||
                   is_solid(right, next_y + PLAYER_HEIGHT)) {
            next_player_y = next_y << FP_SHIFT;
            velocity_y = 0;
            on_ground = 1u;
        }
    } else if (velocity_y < 0) {
        if (is_solid(left, next_y) || is_solid(right, next_y)) {
            next_y = ((next_y >> 3) + 1) << 3;
            next_player_y = next_y << FP_SHIFT;
            velocity_y = 0;
        }
    }

    player_y = next_player_y;
}

static void update_player(uint8_t keys, uint8_t pressed) {
    if (keys & J_LEFT) {
        velocity_x -= WALK_ACCEL;
        if (velocity_x < -WALK_SPEED) velocity_x = -WALK_SPEED;
    } else if (keys & J_RIGHT) {
        velocity_x += WALK_ACCEL;
        if (velocity_x > WALK_SPEED) velocity_x = WALK_SPEED;
    } else if (velocity_x > 0) {
        velocity_x -= WALK_DRAG;
    } else if (velocity_x < 0) {
        velocity_x += WALK_DRAG;
    }

    if ((pressed & J_A) && on_ground) {
        velocity_y = JUMP_SPEED;
        on_ground = 0u;
    }

    velocity_y += GRAVITY;
    if (velocity_y > MAX_FALL_SPEED) velocity_y = MAX_FALL_SPEED;

    update_horizontal();
    update_vertical();
}

static void draw_player(void) {
    uint8_t screen_x = (uint8_t)(player_x >> FP_SHIFT) + 8u;
    uint8_t screen_y = (uint8_t)(player_y >> FP_SHIFT) + 16u;

    move_sprite(0u, screen_x, screen_y);
    move_sprite(1u, screen_x, screen_y + 8u);
}

void main(void) {
    uint8_t keys = 0u;
    uint8_t previous_keys = 0u;

    DISPLAY_OFF;
    BGP_REG = 0xE4u;
    OBP0_REG = 0xE4u;

    build_level();
    set_bkg_data(0u, 2u, background_tiles);
    set_bkg_tiles(0u, 0u, MAP_WIDTH, MAP_HEIGHT, level_map);

    SPRITES_8x8;
    set_sprite_data(0u, 2u, player_tiles);
    set_sprite_tile(0u, 0u);
    set_sprite_tile(1u, 1u);

    player_x = PLAYER_START_X << FP_SHIFT;
    player_y = PLAYER_START_Y << FP_SHIFT;
    velocity_x = 0;
    velocity_y = 0;
    on_ground = 1u;
    draw_player();

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        previous_keys = keys;
        keys = joypad();
        update_player(keys, keys & (uint8_t)~previous_keys);
        draw_player();
        vsync();
    }
}
