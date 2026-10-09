#include "game.h"

enum {
    FieldLeft = 12,
    FieldRight = 468,
    FieldTop = 26,
    BrickColumns = 12,
    BrickRows = 7,
    BrickWidth = 38,
    BrickHeight = 12,
    BrickTop = 46,
    PaddleWidth = 64,
    PaddleHeight = 6,
    PaddleY = 250,
    BallSize = 6,
    MaxParticles = 768,
    StarCount = 140,
    TrailLength = 10,
};

typedef enum Mode {
    ModeAttract,
    ModePlay,
    ModeGameOver,
} Mode;

typedef struct Particle {
    float x, y, vx, vy, life, maxLife;
    uint32_t color;
} Particle;

typedef struct Star {
    float x, y, speed;
    uint32_t color;
} Star;

static const uint8_t font[96][7] = {
    ['!' - 32] = {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04},
    ['\'' - 32] = {0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00},
    ['(' - 32] = {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
    [')' - 32] = {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
    ['+' - 32] = {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
    [',' - 32] = {0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08},
    ['-' - 32] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
    ['.' - 32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C},
    ['/' - 32] = {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10},
    ['0' - 32] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
    ['1' - 32] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    ['2' - 32] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
    ['3' - 32] = {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
    ['4' - 32] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
    ['5' - 32] = {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    ['6' - 32] = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
    ['7' - 32] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    ['8' - 32] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
    ['9' - 32] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
    [':' - 32] = {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00},
    ['<' - 32] = {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
    ['=' - 32] = {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
    ['>' - 32] = {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
    ['?' - 32] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
    ['A' - 32] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    ['B' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
    ['C' - 32] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
    ['D' - 32] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
    ['E' - 32] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
    ['F' - 32] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
    ['G' - 32] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
    ['H' - 32] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    ['I' - 32] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
    ['J' - 32] = {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
    ['K' - 32] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
    ['L' - 32] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
    ['M' - 32] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
    ['N' - 32] = {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11},
    ['O' - 32] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    ['P' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
    ['Q' - 32] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
    ['R' - 32] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
    ['S' - 32] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
    ['T' - 32] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
    ['U' - 32] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    ['V' - 32] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04},
    ['W' - 32] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A},
    ['X' - 32] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
    ['Y' - 32] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
    ['Z' - 32] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
    ['_' - 32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
};

uint32_t canvas[ViewHeight][ViewWidth];

static uint32_t background[ViewHeight];
static uint8_t bricks[BrickRows][BrickColumns];
static Particle particles[MaxParticles];
static Star stars[StarCount];
static float trailX[TrailLength];
static float trailY[TrailLength];

static int bricksLeft;
static int particleCount;
static int trailHead;
static int score;
static int highScore;
static int lives;
static int level;
static Mode mode;
static bool ballHeld;
static float paddleX;
static float ballX;
static float ballY;
static float ballVX;
static float ballVY;
static float ballSpeed;
static float aiOffset;
static float serveTimer;
static float modeTime;
static float levelBanner;
static float shake;
static float elapsed;
static int shakeX;
static int shakeY;
static uint32_t rngState = 0x2F6B1D3Au;

static uint32_t random_next(void)
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

static float random_range(float low, float high)
{
    return low + (high - low) * (float)(random_next() >> 8) * (1.0f / 16777216.0f);
}

static float absf(float value)
{
    return value < 0.0f ? -value : value;
}

static float clampf(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

static float wave(float x)
{
    const float tau = 6.28318531f;
    x -= tau * (float)(int64_t)(x / tau);
    if (x < 0.0f) {
        x += tau;
    }
    if (x > 3.14159265f) {
        x -= tau;
    }
    float y = 1.27323954f * x - 0.40528473f * x * absf(x);
    return 0.225f * (y * absf(y) - y) + y;
}

static uint32_t rgb(uint32_t r, uint32_t g, uint32_t b)
{
    return 0xFF000000u | (b << 16) | (g << 8) | r;
}

static uint32_t mix(uint32_t a, uint32_t b, uint32_t amount)
{
    uint32_t inverse = 256 - amount;
    uint32_t r = ((a & 0xFF) * inverse + (b & 0xFF) * amount) >> 8;
    uint32_t g = (((a >> 8) & 0xFF) * inverse + ((b >> 8) & 0xFF) * amount) >> 8;
    uint32_t bl = (((a >> 16) & 0xFF) * inverse + ((b >> 16) & 0xFF) * amount) >> 8;
    return rgb(r, g, bl);
}

static uint32_t white(void)
{
    return rgb(255, 255, 255);
}

static uint32_t electric(void)
{
    return rgb(0, 71, 239);
}

static uint32_t neon(void)
{
    return rgb(51, 119, 249);
}

static uint32_t ice(void)
{
    return rgb(176, 208, 240);
}

static uint32_t sky(void)
{
    return rgb(32, 144, 240);
}

static uint32_t purple(void)
{
    return rgb(144, 96, 240);
}

static uint32_t orchid(void)
{
    return rgb(192, 128, 240);
}

static uint32_t field_color(int x, int y)
{
    int position = x + y;
    position = position < 0 ? 0 : position >= ViewWidth + ViewHeight ? ViewWidth + ViewHeight - 1 : position;
    return mix(sky(), purple(), (uint32_t)(position * 256 / (ViewWidth + ViewHeight)));
}

static uint32_t brick_color(int row, int column)
{
    return field_color(FieldLeft + column * BrickWidth + BrickWidth / 2, BrickTop + row * BrickHeight + BrickHeight / 2);
}

static void fill_rect(int x, int y, int width, int height, uint32_t color)
{
    int right = x + width;
    int bottom = y + height;
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    right = right > ViewWidth ? ViewWidth : right;
    bottom = bottom > ViewHeight ? ViewHeight : bottom;
    for (int row = y; row < bottom; row++) {
        for (int column = x; column < right; column++) {
            canvas[row][column] = color;
        }
    }
}

static void blend_rect(int x, int y, int width, int height, uint32_t color, uint32_t amount)
{
    int right = x + width;
    int bottom = y + height;
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    right = right > ViewWidth ? ViewWidth : right;
    bottom = bottom > ViewHeight ? ViewHeight : bottom;
    for (int row = y; row < bottom; row++) {
        for (int column = x; column < right; column++) {
            canvas[row][column] = mix(canvas[row][column], color, amount);
        }
    }
}

static void gradient_rect(int x, int y, int width, int height, uint32_t amount)
{
    int right = x + width;
    int bottom = y + height;
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    right = right > ViewWidth ? ViewWidth : right;
    bottom = bottom > ViewHeight ? ViewHeight : bottom;
    for (int row = y; row < bottom; row++) {
        for (int column = x; column < right; column++) {
            canvas[row][column] = mix(canvas[row][column], field_color(column, row), amount);
        }
    }
}

static void glow_rect(int x, int y, int width, int height, uint32_t color, uint32_t core)
{
    blend_rect(x - 3, y - 3, width + 6, height + 6, color, core / 6);
    blend_rect(x - 1, y - 1, width + 2, height + 2, color, core / 2);
}

static int text_length(const char *text)
{
    int length = 0;
    while (text[length]) {
        length++;
    }
    return length;
}

static int text_width(const char *text, int scale)
{
    return text_length(text) * 6 * scale - scale;
}

static void draw_glyph(int x, int y, char character, int scale, uint32_t color, bool glow)
{
    if (character >= 'a' && character <= 'z') {
        character = (char)(character - 32);
    }
    if (character < 32 || character > 127) {
        return;
    }
    const uint8_t *rows = font[character - 32];
    for (int row = 0; row < 7; row++) {
        for (int column = 0; column < 5; column++) {
            if (!(rows[row] & (0x10 >> column))) {
                continue;
            }
            int left = x + column * scale;
            int top = y + row * scale;
            if (glow) {
                blend_rect(left - 2, top - 2, scale + 4, scale + 4, electric(), 34);
                blend_rect(left - 1, top - 1, scale + 2, scale + 2, neon(), 60);
            } else {
                fill_rect(left, top, scale, scale, color);
            }
        }
    }
}

static void draw_text(int x, int y, const char *text, int scale, uint32_t color)
{
    for (int index = 0; text[index]; index++) {
        draw_glyph(x + index * 6 * scale, y, text[index], scale, color, false);
    }
}

static void draw_text_centered(int y, const char *text, int scale, uint32_t color)
{
    draw_text((ViewWidth - text_width(text, scale)) / 2, y, text, scale, color);
}

static void draw_wave_text(int y, const char *text, int scale, float amplitude)
{
    int x = (ViewWidth - text_width(text, scale)) / 2;
    for (int pass = 0; pass < 2; pass++) {
        for (int index = 0; text[index]; index++) {
            int offset = (int)(wave(elapsed * 3.0f + (float)index * 0.55f) * amplitude);
            uint32_t color = mix(white(), ice(), (uint32_t)(110.0f + 110.0f * wave(elapsed * 2.4f + (float)index * 0.5f)));
            draw_glyph(x + index * 6 * scale, y + offset, text[index], scale, color, pass == 0);
        }
    }
}

static char *append_text(char *destination, const char *text)
{
    while (*text) {
        *destination++ = *text++;
    }
    *destination = 0;
    return destination;
}

static char *append_number(char *destination, uint32_t value, int digits)
{
    char reversed[16];
    int count = 0;
    do {
        reversed[count++] = (char)('0' + value % 10);
        value /= 10;
    } while (value && count < 15);
    while (count < digits && count < 15) {
        reversed[count++] = '0';
    }
    while (count) {
        *destination++ = reversed[--count];
    }
    *destination = 0;
    return destination;
}

static void init_scenery(void)
{
    for (int y = 0; y < ViewHeight; y++) {
        background[y] = mix(rgb(0, 0, 0), rgb(8, 6, 26), (uint32_t)(y * 256 / ViewHeight));
    }
    for (int index = 0; index < StarCount; index++) {
        float depth = random_range(0.2f, 1.0f);
        uint32_t shade = (uint32_t)(40.0f + 150.0f * depth);
        stars[index].x = random_range(0.0f, (float)ViewWidth);
        stars[index].y = random_range(0.0f, (float)ViewHeight);
        stars[index].speed = 6.0f + 26.0f * depth;
        stars[index].color = rgb(shade * 3 / 4, shade * 7 / 8, shade);
    }
}

static void burst(float x, float y, uint32_t color, int count, float speed)
{
    for (int index = 0; index < count; index++) {
        if (particleCount >= MaxParticles) {
            return;
        }
        Particle *particle = &particles[particleCount++];
        float angle = random_range(0.0f, 6.2831853f);
        float velocity = random_range(0.3f, 1.0f) * speed;
        particle->x = x;
        particle->y = y;
        particle->vx = wave(angle + 1.5707963f) * velocity;
        particle->vy = wave(angle) * velocity - speed * 0.4f;
        particle->maxLife = random_range(0.45f, 0.95f);
        particle->life = particle->maxLife;
        particle->color = color;
    }
}

static void reset_bricks(void)
{
    for (int row = 0; row < BrickRows; row++) {
        for (int column = 0; column < BrickColumns; column++) {
            bricks[row][column] = 1;
        }
    }
    bricksLeft = BrickRows * BrickColumns;
}

static void reset_trail(void)
{
    for (int index = 0; index < TrailLength; index++) {
        trailX[index] = ballX;
        trailY[index] = ballY;
    }
}

static void hold_ball(void)
{
    ballHeld = true;
    serveTimer = 0.0f;
    ballX = paddleX + (PaddleWidth - BallSize) * 0.5f;
    ballY = PaddleY - BallSize - 1;
    reset_trail();
}

static void aim_ball(float direction)
{
    direction = clampf(direction, -0.82f, 0.82f);
    ballVX = ballSpeed * direction;
    ballVY = -__builtin_sqrtf(ballSpeed * ballSpeed - ballVX * ballVX);
}

static void start_game(Mode next)
{
    mode = next;
    modeTime = 0.0f;
    score = 0;
    lives = 3;
    level = 1;
    ballSpeed = 200.0f;
    paddleX = (FieldLeft + FieldRight - PaddleWidth) * 0.5f;
    levelBanner = next == ModePlay ? 2.0f : 0.0f;
    reset_bricks();
    hold_ball();
}

static void lose_ball(void)
{
    burst(ballX, (float)ViewHeight - 4.0f, neon(), 30, 150.0f);
    shake = 4.0f;
    lives--;
    if (lives > 0) {
        hold_ball();
        return;
    }
    if (mode == ModePlay) {
        highScore = score > highScore ? score : highScore;
        mode = ModeGameOver;
        modeTime = 0.0f;
        ballHeld = true;
        ballY = (float)ViewHeight + 20.0f;
        return;
    }
    start_game(ModeAttract);
}

static void normalize_ball(void)
{
    float length = __builtin_sqrtf(ballVX * ballVX + ballVY * ballVY);
    if (length < 1.0f) {
        aim_ball(0.3f);
        return;
    }
    ballVX = ballVX / length * ballSpeed;
    ballVY = ballVY / length * ballSpeed;
    float minimum = ballSpeed * 0.35f;
    if (absf(ballVY) < minimum) {
        ballVY = ballVY < 0.0f ? -minimum : minimum;
        float side = __builtin_sqrtf(ballSpeed * ballSpeed - minimum * minimum);
        ballVX = ballVX < 0.0f ? -side : side;
    }
}

static void break_brick(int row, int column)
{
    bricks[row][column] = 0;
    bricksLeft--;
    score += (BrickRows - row) * 10 * level;
    uint32_t color = brick_color(row, column);
    float x = FieldLeft + column * BrickWidth + BrickWidth * 0.5f;
    float y = BrickTop + row * BrickHeight + BrickHeight * 0.5f;
    burst(x, y, color, 18, 120.0f);
    platform_set_light_bar(color);
    shake = shake < 1.5f ? 1.5f : shake;
    ballSpeed = clampf(ballSpeed + 1.5f, 0.0f, 340.0f + level * 20.0f);
    if (bricksLeft == 0) {
        level++;
        ballSpeed = 200.0f + (float)(level - 1) * 18.0f;
        levelBanner = 2.0f;
        reset_bricks();
        hold_ball();
    }
}

static bool collide_bricks(void)
{
    int firstColumn = (int)((ballX - FieldLeft) / BrickWidth);
    int lastColumn = (int)((ballX + BallSize - 1 - FieldLeft) / BrickWidth);
    int firstRow = (int)((ballY - BrickTop) / BrickHeight);
    int lastRow = (int)((ballY + BallSize - 1 - BrickTop) / BrickHeight);
    if (ballY + BallSize < BrickTop || ballY >= BrickTop + BrickRows * BrickHeight) {
        return false;
    }
    for (int row = firstRow; row <= lastRow; row++) {
        for (int column = firstColumn; column <= lastColumn; column++) {
            if (row < 0 || row >= BrickRows || column < 0 || column >= BrickColumns || !bricks[row][column]) {
                continue;
            }
            float left = (float)(FieldLeft + column * BrickWidth);
            float top = (float)(BrickTop + row * BrickHeight);
            float overlapX = (ballX + BallSize < left + BrickWidth ? ballX + BallSize : left + BrickWidth) -
                             (ballX > left ? ballX : left);
            float overlapY = (ballY + BallSize < top + BrickHeight ? ballY + BallSize : top + BrickHeight) -
                             (ballY > top ? ballY : top);
            if (overlapX < overlapY) {
                ballX += ballX + BallSize * 0.5f < left + BrickWidth * 0.5f ? -overlapX : overlapX;
                ballVX = -ballVX;
            } else {
                ballY += ballY + BallSize * 0.5f < top + BrickHeight * 0.5f ? -overlapY : overlapY;
                ballVY = -ballVY;
            }
            break_brick(row, column);
            normalize_ball();
            return true;
        }
    }
    return false;
}

static void step_ball(float dt)
{
    if (ballHeld) {
        ballX = paddleX + (PaddleWidth - BallSize) * 0.5f;
        if (mode != ModeGameOver) {
            ballY = PaddleY - BallSize - 1;
        }
        return;
    }

    ballX += ballVX * dt;
    ballY += ballVY * dt;
    if (ballX < FieldLeft) {
        ballX = FieldLeft;
        ballVX = absf(ballVX);
        normalize_ball();
    }
    if (ballX + BallSize > FieldRight) {
        ballX = FieldRight - BallSize;
        ballVX = -absf(ballVX);
        normalize_ball();
    }
    if (ballY < FieldTop) {
        ballY = FieldTop;
        ballVY = absf(ballVY);
    }

    if (ballVY > 0.0f && ballY + BallSize >= PaddleY && ballY + BallSize <= PaddleY + PaddleHeight + 5 &&
        ballX + BallSize >= paddleX && ballX <= paddleX + PaddleWidth) {
        float hit = ((ballX + BallSize * 0.5f) - (paddleX + PaddleWidth * 0.5f)) / (PaddleWidth * 0.5f);
        ballY = PaddleY - BallSize;
        aim_ball(hit);
        burst(ballX + BallSize * 0.5f, (float)PaddleY, ice(), 6, 60.0f);
        aiOffset = random_range(-22.0f, 22.0f);
    }

    collide_bricks();
    if (ballY > ViewHeight + 8) {
        lose_ball();
    }
}

static void move_paddle(float dt, const Input *input)
{
    if (mode == ModePlay) {
        paddleX += input->stick * 430.0f * dt;
    } else {
        float aim = ballVY > 0.0f ? ballX + BallSize * 0.5f + aiOffset : (ballX + (FieldLeft + FieldRight) * 0.5f) * 0.5f;
        float target = aim - PaddleWidth * 0.5f;
        float step = 330.0f * dt;
        paddleX += clampf(target - paddleX, -step, step);
    }
    paddleX = clampf(paddleX, (float)FieldLeft, (float)(FieldRight - PaddleWidth));
}

static void advance_particles(float dt)
{
    for (int index = 0; index < particleCount;) {
        Particle *particle = &particles[index];
        particle->life -= dt;
        if (particle->life <= 0.0f) {
            *particle = particles[--particleCount];
            continue;
        }
        particle->vy += 260.0f * dt;
        particle->x += particle->vx * dt;
        particle->y += particle->vy * dt;
        index++;
    }
}

void game_update(float dt, const Input *input)
{
    elapsed += dt;
    modeTime += dt;
    levelBanner = levelBanner > 0.0f ? levelBanner - dt : 0.0f;
    shake = shake > 0.0f ? shake - dt * 12.0f : 0.0f;

    if (mode == ModeAttract && input->cross) {
        start_game(ModePlay);
    } else if (mode == ModePlay && input->options) {
        start_game(ModeAttract);
    } else if (mode == ModeGameOver && (modeTime > 4.0f || (modeTime > 1.0f && input->cross))) {
        start_game(ModeAttract);
    }

    if (ballHeld && mode != ModeGameOver) {
        serveTimer += dt;
        bool serve = mode == ModePlay ? input->cross || serveTimer > 3.0f : serveTimer > 0.8f;
        if (serve && levelBanner < 1.2f) {
            ballHeld = false;
            aim_ball(random_range(-0.45f, 0.45f));
        }
    }

    const float stepSize = 1.0f / 240.0f;
    int steps = (int)(dt / stepSize) + 1;
    steps = steps > 40 ? 40 : steps;
    for (int step = 0; step < steps; step++) {
        move_paddle(dt / steps, input);
        step_ball(dt / steps);
    }

    trailHead = (trailHead + 1) % TrailLength;
    trailX[trailHead] = ballX;
    trailY[trailHead] = ballY;

    advance_particles(dt);

    for (int index = 0; index < StarCount; index++) {
        stars[index].y += stars[index].speed * dt;
        if (stars[index].y >= ViewHeight) {
            stars[index].y -= ViewHeight;
            stars[index].x = random_range(0.0f, (float)ViewWidth);
        }
    }

    int amount = (int)(shake + 0.5f);
    shakeX = amount ? (int)(random_next() % (uint32_t)(amount * 2 + 1)) - amount : 0;
    shakeY = amount ? (int)(random_next() % (uint32_t)(amount * 2 + 1)) - amount : 0;
}

static void draw_frame(void)
{
    int left = FieldLeft - 2 + shakeX;
    int top = FieldTop - 2 + shakeY;
    int right = FieldRight + shakeX;
    int width = FieldRight - FieldLeft + 4;
    for (int ring = 3; ring >= 0; ring--) {
        uint32_t amount = ring == 0 ? 256 : (uint32_t)(80 / ring);
        gradient_rect(left - ring, top - ring, width + ring * 2, 2 + ring * 2, amount);
        gradient_rect(left - ring, top + 2 + ring, 2 + ring * 2, ViewHeight, amount);
        gradient_rect(right - ring, top + 2 + ring, 2 + ring * 2, ViewHeight, amount);
    }
}

static void draw_neon_brick(int x, int y, int width, int height, uint32_t color)
{
    blend_rect(x - 1, y - 1, width + 2, height + 2, color, 70);
    fill_rect(x, y, width, height, mix(rgb(0, 0, 0), color, 52));
    fill_rect(x, y, width, 1, mix(color, white(), 120));
    fill_rect(x, y + height - 1, width, 1, color);
    fill_rect(x, y, 1, height, color);
    fill_rect(x + width - 1, y, 1, height, color);
}

static void draw_bricks(void)
{
    for (int row = 0; row < BrickRows; row++) {
        for (int column = 0; column < BrickColumns; column++) {
            if (bricks[row][column]) {
                draw_neon_brick(FieldLeft + column * BrickWidth + 1 + shakeX, BrickTop + row * BrickHeight + 1 + shakeY,
                                BrickWidth - 2, BrickHeight - 2, brick_color(row, column));
            }
        }
    }
}

static void draw_particles(void)
{
    for (int index = 0; index < particleCount; index++) {
        Particle *particle = &particles[index];
        uint32_t amount = (uint32_t)(256.0f * particle->life / particle->maxLife);
        blend_rect((int)particle->x + shakeX, (int)particle->y + shakeY, 2, 2, particle->color, amount);
    }
}

static void draw_trail_dot(float x, float y, int age)
{
    uint32_t amount = (uint32_t)(170 * (TrailLength - age) / TrailLength);
    blend_rect((int)x + 1 + shakeX, (int)y + 1 + shakeY, BallSize - 2, BallSize - 2, neon(), amount);
}

static void draw_ball(int x, int y)
{
    glow_rect(x, y, BallSize, BallSize, neon(), 220);
    fill_rect(x, y, BallSize, BallSize, white());
}

static void draw_paddle(int x, int y, int width)
{
    glow_rect(x, y, width, PaddleHeight, electric(), 240);
    fill_rect(x, y, width, PaddleHeight, white());
    fill_rect(x + 2, y + 2, width - 4, PaddleHeight - 4, mix(rgb(0, 0, 0), electric(), 120));
}

static void draw_background(void)
{
    for (int y = 0; y < ViewHeight; y++) {
        uint32_t color = background[y];
        for (int x = 0; x < ViewWidth; x++) {
            canvas[y][x] = color;
        }
    }
    for (int index = 0; index < StarCount; index++) {
        canvas[(int)stars[index].y][(int)stars[index].x] = stars[index].color;
    }
}

static void draw_hud(void)
{
    char text[48];
    char *end = append_text(text, "SCORE ");
    append_number(end, (uint32_t)score, 6);
    draw_text(FieldLeft, 6, text, 2, white());

    end = append_text(text, "HI ");
    append_number(end, (uint32_t)highScore, 6);
    draw_text(FieldRight - text_width(text, 2), 6, text, 2, orchid());

    if (mode == ModePlay) {
        for (int index = 0; index < lives; index++) {
            glow_rect(ViewWidth / 2 - lives * 7 + index * 14 + 2, 10, 8, 8, neon(), 200);
            fill_rect(ViewWidth / 2 - lives * 7 + index * 14 + 2, 10, 8, 8, white());
        }
    } else if (mode == ModeAttract) {
        draw_text_centered(9, "DEMO", 1, rgb(96, 110, 150));
    }
}

static void draw_overlay(void)
{
    if (mode == ModeAttract) {
        blend_rect(0, 128, ViewWidth, 96, rgb(0, 0, 0), 200);
        draw_wave_text(138, "SHARPEMU", 5, 4.0f);
        draw_text_centered(184, "HELLO, WORLD! THIS ELF RUNS ON SHARPEMU.", 1, ice());
        if ((int)(elapsed * 2.0f) % 2 == 0) {
            draw_text_centered(200, "PRESS CROSS TO PLAY", 2, white());
        }
    } else if (mode == ModeGameOver) {
        char text[32];
        blend_rect(0, 132, ViewWidth, 76, rgb(0, 0, 0), 200);
        draw_wave_text(142, "GAME OVER", 4, 3.0f);
        append_number(append_text(text, "SCORE "), (uint32_t)score, 6);
        draw_text_centered(184, text, 2, orchid());
    } else if (levelBanner > 0.0f) {
        char text[16];
        append_number(append_text(text, "LEVEL "), (uint32_t)level, 1);
        draw_wave_text(160, text, 4, 3.0f);
    } else if (ballHeld) {
        draw_text_centered(214, "PRESS CROSS TO LAUNCH", 1, ice());
    }
}

void game_render(int framesPerSecond)
{
    draw_background();
    draw_frame();
    draw_bricks();
    draw_particles();

    if (mode != ModeGameOver) {
        for (int age = TrailLength - 1; age > 0; age--) {
            int slot = (trailHead - age + TrailLength) % TrailLength;
            draw_trail_dot(trailX[slot], trailY[slot], age);
        }
        draw_ball((int)ballX + shakeX, (int)ballY + shakeY);
        draw_paddle((int)paddleX + shakeX, PaddleY + shakeY, PaddleWidth);
    }

    draw_hud();
    draw_overlay();

    char text[16];
    append_text(append_number(text, (uint32_t)framesPerSecond, 1), " FPS");
    draw_text(ViewWidth - text_width(text, 1) - 4, ViewHeight - 10, text, 1, rgb(60, 72, 120));
}

void game_init(uint32_t seed)
{
    rngState ^= seed | 1u;
    init_scenery();
    start_game(ModeAttract);
}
