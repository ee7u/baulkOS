#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef struct FrameBuffer {
    void* buffer;
    u64 size;
    u64 width;
    u64 height;
    u64 stride;
} FrameBuffer;

typedef struct PSF1Header {
    u8 magic[2];
    u8 mode;
    u8 charSize;
} PSF1Header;

typedef struct PSF1Font {
    PSF1Header* header;
    void* buffer;
} PSF1Font;

static FrameBuffer FB;
static PSF1Font FONT;

#define WHITE 0xffffff
#define RED 0xff0000

typedef struct limine_file LimineFile;

typedef struct String8 {
    u8* str;
    u64 size;
} String8;

String8 str8(u8* str, u64 size) {
    String8 ret = {str, size};
    return ret;
}

#define str8_lit(S) str8((u8*)(S), sizeof(S) - 1)

String8 u64_to_str8(u64 x, u8* buffer, u64 bufferSize) {
    u64 i = 0;
    while (x != 0) {
        u8 digit = x % 10;
        buffer[bufferSize - i - 1] = '0' + digit;
        i++;
        x /= 10;
    }
    if (i == 0) {
        buffer[bufferSize - 1] = '0';
        i++;
    }
    String8 ret = {buffer + bufferSize - i, i};
    return ret;
}

String8 u64_to_str8_hex(u64 x, u8* buffer, u64 bufferSize) {
    u64 i = 0;
    while (x != 0) {
        u8 digit = x % 16;
        if (digit < 10) {
            buffer[bufferSize - i - 1] = '0' + digit;
        } else {
            buffer[bufferSize - i - 1] = 'a' + digit - 10;
        }
        i++;
        x /= 16;
    }
    buffer[bufferSize - i - 1] = 'x';
    buffer[bufferSize - i - 2] = '0';
    i += 2;
    String8 ret = {buffer + bufferSize - i, i};
    return ret;
}

String8 i64_to_str8(i64 x, u8* buffer, u64 bufferSize) {
    i8 sign = x >= 0 ? 1 : -1;
    if (sign == -1) {
        x *= -1;
    }

    String8 strUnsigned = u64_to_str8(x, buffer, bufferSize);
    if (sign == 1) {
        return strUnsigned;
    }

    buffer[bufferSize - strUnsigned.size - 1] = '-';
    String8 ret = {buffer + bufferSize - strUnsigned.size - 1, strUnsigned.size + 1};
    return ret;
}

bool str8_eq(String8 a, String8 b) {
    if (a.size != b.size) {
        return false;
    }

    for (u64 i = 0; i < a.size; i++) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
    }

    return true;
}

String8 str8_suffix(String8 str, u64 n) {
    String8 ret = {str.str + str.size - n, n};
    return ret;
}

String8 str8_from_cstr(const char* cstr) {
    u64 strLen = 0;
    while (cstr[strLen] != 0) {
        strLen++;
    }
    String8 ret = {(u8*)cstr, strLen};
    return ret;
}

LimineFile* limine_get_file(String8 name) {
    struct limine_module_response* response = module_request.response;

    if (response == NULL) {
        hcf();
    }

    for (u64 i = 0; i < response->module_count; i++) {
        LimineFile* f = response->modules[i];
        String8 filePath = str8_from_cstr(f->path);
        if (str8_eq(name, str8_suffix(filePath, name.size))) {
            return f;
        }
    }

    return NULL;
}

static u64 cursor_x = 0;
static u64 cursor_y = 0;

void put_char(char c, u32 color) {
    u32* frameBuffer = (u32*)FB.buffer;
    char* fontBuffer = (char*)FONT.buffer + (c * FONT.header->charSize);

    for (u64 y = cursor_y; y < cursor_y + 16; y++) {
        for (u64 x = cursor_x; x < cursor_x + 8; x++) {
            if ((fontBuffer[y-cursor_y] & (0b10000000 >> (x - cursor_x))) > 0) {
                *(u32*)(frameBuffer + x + (y * FB.width)) = color;
            }
        }
    }
}

#define CHAR_HEIGHT 16
#define CHAR_WIDTH 8

void print_color(String8 str, u32 color) {
    for (u64 i = 0; i < str.size; i++) {
        if (str.str[i] == '\n') {
            cursor_x = 0;
            cursor_y += CHAR_HEIGHT;
        } else {
            put_char(str.str[i], color);
            cursor_x += CHAR_WIDTH;
        }

        if (cursor_x + CHAR_WIDTH > FB.width) {
            cursor_x = 0;
            cursor_y += CHAR_HEIGHT;
        }

        if (cursor_y + CHAR_HEIGHT > FB.height) {
            // scroll
            memmove(FB.buffer, FB.buffer + CHAR_HEIGHT*FB.stride, FB.size - CHAR_HEIGHT*FB.stride - 1);
            memset(FB.buffer + FB.size - CHAR_HEIGHT*FB.stride, 0, CHAR_HEIGHT*FB.stride - 1);
            cursor_y -= CHAR_HEIGHT;
            cursor_x = 0;
        }
    }
}

void print(String8 str) {
    print_color(str, WHITE);
}

void print_err(String8 str) {
    print_color(str, RED);
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
         hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    FB.buffer = framebuffer->address;
    FB.width = framebuffer->width;
    FB.height = framebuffer->height;
    FB.size = framebuffer->height * framebuffer->pitch;
    FB.stride = framebuffer->pitch;

    String8 fName = str8_lit("zap-light16.psf");
    LimineFile* file = limine_get_file(fName);
    if (file == NULL) {
        hcf();
    }

    FONT.header = file->address;
    if (FONT.header->magic[0] != 0x36 || FONT.header->magic[1] != 0x04) {
        hcf();
    }
    FONT.buffer = (void*)((u64)file->address + sizeof(PSF1Header));

    u8 buffer[128];
    print(u64_to_str8(272, buffer, 128));
    print(str8_lit("\n"));
    print(u64_to_str8_hex(272, buffer, 128));
    print(str8_lit("\n"));
    print(i64_to_str8(11, buffer, 128));
    print(str8_lit("\n"));
    print(i64_to_str8(-11, buffer, 128));
    print(str8_lit("\n"));
    print(i64_to_str8(0, buffer, 128));
    print(str8_lit("\n"));
    print(u64_to_str8_hex((u64)&FB, buffer, 128));
    print(str8_lit("\n"));
    print(u64_to_str8_hex((u64)&file, buffer, 128));
    print(str8_lit("\n"));


    hcf();
}
