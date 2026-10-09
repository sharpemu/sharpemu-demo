#include <stddef.h>

#include "game.h"

typedef struct VideoBuffer {
    void *data;
    uint64_t reserved[3];
} VideoBuffer;

typedef struct VideoAttribute {
    uint8_t reserved[80];
} VideoAttribute;

typedef struct KernelEvent {
    uint64_t ident;
    int16_t filter;
    uint16_t flags;
    uint32_t fflags;
    int64_t data;
    void *udata;
} KernelEvent;

typedef struct UserServiceParams {
    int32_t priority;
} UserServiceParams;

typedef struct PadColor {
    uint8_t r, g, b, a;
} PadColor;

typedef struct PadData {
    uint32_t buttons;
    uint8_t leftX, leftY, rightX, rightY;
    uint8_t rest[112];
} PadData;

int sceSystemServiceHideSplashScreen(void);
int sceKernelAllocateMainDirectMemory(size_t, size_t, int, intptr_t *);
int sceKernelMapDirectMemory(void **, size_t, int, int, intptr_t, size_t);
int sceKernelUsleep(unsigned int);
uint64_t sceKernelGetProcessTime(void);
int sceKernelCreateEqueue(void **, const char *);
int sceKernelWaitEqueue(void *, KernelEvent *, int, int *, unsigned int *);
int sceVideoOutOpen(int, int, int, const void *);
int sceVideoOutSetFlipRate(int, int);
int sceVideoOutAddFlipEvent(void *, int, void *);
int sceVideoOutSubmitFlip(int, int, uint32_t, int64_t);
void sceVideoOutSetBufferAttribute2(VideoAttribute *, uint64_t, uint32_t, uint32_t, uint32_t, uint64_t, uint32_t, uint64_t);
int sceVideoOutRegisterBuffers2(int, int, int, VideoBuffer *, int, VideoAttribute *, int, void *);
int sceUserServiceInitialize(const UserServiceParams *);
int sceUserServiceGetInitialUser(int32_t *);
int scePadInit(void);
int scePadOpen(int32_t, int32_t, int32_t, const void *);
int scePadReadState(int32_t, PadData *);
int scePadSetLightBar(int32_t, const PadColor *);

void *memset(void *destination, int value, size_t size)
{
    uint8_t *bytes = destination;
    while (size--) {
        *bytes++ = (uint8_t)value;
    }
    return destination;
}

void *memcpy(void *destination, const void *source, size_t size)
{
    uint8_t *to = destination;
    const uint8_t *from = source;
    while (size--) {
        *to++ = *from++;
    }
    return destination;
}

enum {
    Scale = 4,
    ScreenWidth = ViewWidth * Scale,
    ScreenHeight = ViewHeight * Scale,
    TileWidth = 512,
    TileHeight = 128,
};

enum {
    ButtonOptions = 0x0008,
    ButtonRight = 0x0020,
    ButtonLeft = 0x0080,
    ButtonCross = 0x4000,
};

static uint32_t columnBase[ViewWidth];
static uint32_t columnSwizzle[ViewWidth];
static int32_t padHandle = -1;
static uint32_t previousButtons;

static uint32_t tile_offset(uint32_t x, uint32_t y)
{
    return ((x & 1u) << 0) | (((x >> 1) & 1u) << 1) | ((y & 1u) << 2) | (((y >> 1) & 1u) << 3) |
           (((y >> 2) & 1u) << 4) | (((x >> 2) & 1u) << 5) | ((((x >> 3) ^ (y >> 3)) & 1u) << 6) |
           ((((x >> 4) ^ (y >> 4)) & 1u) << 7) | ((((x >> 6) ^ (y >> 5)) & 1u) << 8) |
           ((((x >> 5) ^ (y >> 6)) & 1u) << 9) | (((y >> 3) & 1u) << 10) | (((x >> 4) & 1u) << 11) |
           (((y >> 6) & 1u) << 12) | (((x >> 6) & 1u) << 13) | (((x >> 7) & 1u) << 14) | (((x >> 8) & 1u) << 15);
}

static void init_tiling(void)
{
    for (uint32_t column = 0; column < ViewWidth; column++) {
        uint32_t x = column * Scale;
        columnBase[column] = (x / TileWidth) * (TileWidth * TileHeight);
        columnSwizzle[column] = tile_offset(x % TileWidth, 0);
    }
}

static void present_canvas(uint32_t *target)
{
    for (uint32_t y = 0; y < ScreenHeight; y++) {
        const uint32_t *source = canvas[y / Scale];
        uint32_t *stripe = target + (y / TileHeight) * (TileHeight * ScreenWidth);
        uint32_t rowSwizzle = tile_offset(0, y % TileHeight);
        for (uint32_t column = 0; column < ViewWidth; column++) {
            uint64_t pair = source[column] | ((uint64_t)source[column] << 32);
            uint64_t *pixels = (uint64_t *)(stripe + columnBase[column] + (columnSwizzle[column] ^ rowSwizzle));
            pixels[0] = pair;
            pixels[1] = pair;
        }
    }
}

void platform_set_light_bar(uint32_t color)
{
    if (padHandle < 0) {
        return;
    }
    PadColor padColor = {(uint8_t)color, (uint8_t)(color >> 8), (uint8_t)(color >> 16), 0};
    scePadSetLightBar(padHandle, &padColor);
}

static void open_pad(void)
{
    UserServiceParams parameters = {256};
    int32_t user = -1;
    sceUserServiceInitialize(&parameters);
    if (sceUserServiceGetInitialUser(&user) != 0 || scePadInit() != 0) {
        return;
    }
    int handle = scePadOpen(user, 0, 0, 0);
    padHandle = handle < 0 ? -1 : handle;
    platform_set_light_bar(0xFFEF4700u);
}

static void read_input(Input *input)
{
    input->stick = 0.0f;
    input->cross = false;
    input->options = false;
    if (padHandle < 0) {
        return;
    }
    PadData data;
    if (scePadReadState(padHandle, &data) != 0) {
        return;
    }
    int axis = (int)data.leftX - 128;
    if (axis > 24 || axis < -24) {
        float stick = (float)axis / 110.0f;
        input->stick = stick > 1.0f ? 1.0f : stick < -1.0f ? -1.0f : stick;
    }
    if (data.buttons & ButtonLeft) {
        input->stick = -1.0f;
    }
    if (data.buttons & ButtonRight) {
        input->stick = 1.0f;
    }
    input->cross = (data.buttons & ButtonCross) && !(previousButtons & ButtonCross);
    input->options = (data.buttons & ButtonOptions) && !(previousButtons & ButtonOptions);
    previousButtons = data.buttons;
}

int demo_main(void)
{
    static VideoBuffer buffers[2];
    static VideoAttribute attribute;
    intptr_t physical = 0;
    void *memory = 0;
    void *flipQueue = 0;
    const size_t size = 0x4000000;

    sceSystemServiceHideSplashScreen();
    open_pad();

    int video = sceVideoOutOpen(0xFF, 0, 0, 0);
    if (video < 0) {
        return 1;
    }
    if (sceKernelAllocateMainDirectMemory(size, 0x20000, 3, &physical) ||
        sceKernelMapDirectMemory(&memory, size, 0x33, 0, physical, 0x20000)) {
        return 2;
    }
    buffers[0].data = memory;
    buffers[1].data = (uint8_t *)memory + size / 2;
    sceVideoOutSetFlipRate(video, 0);
    sceVideoOutSetBufferAttribute2(&attribute, 0x8000000022000000UL, 0, ScreenWidth, ScreenHeight, 0, 0, 0);
    if (sceVideoOutRegisterBuffers2(video, 0, 0, buffers, 2, &attribute, 0, 0)) {
        return 3;
    }
    if (sceKernelCreateEqueue(&flipQueue, "demo_flip") != 0 || sceVideoOutAddFlipEvent(flipQueue, video, 0) != 0) {
        flipQueue = 0;
    }

    uint64_t last = sceKernelGetProcessTime();
    init_tiling();
    game_init((uint32_t)last);

    uint64_t fpsStart = last;
    int fpsFrames = 0;
    int framesPerSecond = 60;
    for (uint32_t frame = 0;; frame++) {
        uint64_t now = sceKernelGetProcessTime();
        float dt = (float)(now - last) * 1e-6f;
        last = now;
        dt = dt > 0.1f ? 0.1f : dt;

        Input input;
        read_input(&input);
        game_update(dt, &input);
        game_render(framesPerSecond);

        int index = (int)(frame & 1);
        present_canvas((uint32_t *)buffers[index].data);
        sceVideoOutSubmitFlip(video, index, 1, frame);
        if (flipQueue) {
            KernelEvent event;
            int count = 0;
            unsigned int timeout = 100000;
            sceKernelWaitEqueue(flipQueue, &event, 1, &count, &timeout);
        } else {
            sceKernelUsleep(16000);
        }

        fpsFrames++;
        if (now - fpsStart >= 1000000) {
            framesPerSecond = (int)((uint64_t)fpsFrames * 1000000 / (now - fpsStart));
            fpsFrames = 0;
            fpsStart = now;
        }
    }
}

__attribute__((naked)) void _start(void)
{
    __asm__ volatile(
        "and $-16, %rsp\n"
        "call demo_main\n"
        "1: pause\n"
        "jmp 1b\n");
}
