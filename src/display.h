#pragma once

#include "string.h"

void put_char(char c, u32 color);

#define CHAR_HEIGHT 16
#define CHAR_WIDTH 8

void print_color(String8 str, u32 color);

void print(String8 str);

void print_err(String8 str);

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

#define WHITE 0xffffff
#define RED 0xff0000

void init_display();
