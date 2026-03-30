#include "display.h"
#include "limine.h"
#include "utils.h"

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

static u64 cursor_x = 0;
static u64 cursor_y = 0;
static FrameBuffer FB;
static PSF1Font FONT;

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

static struct limine_file* limine_get_file(String8 name) {
    struct limine_module_response* response = module_request.response;

    if (response == NULL) {
        hcf();
    }

    for (u64 i = 0; i < response->module_count; i++) {
        struct limine_file* f = response->modules[i];
        String8 filePath = str8_from_cstr(f->path);
        if (str8_eq(name, str8_suffix(filePath, name.size))) {
            return f;
        }
    }

    return NULL;
}


void init_display() {
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

    struct limine_file* file = limine_get_file(str8_lit("Lat38-VGA16.psf"));
    if (file == NULL) {
        hcf();
    }

    FONT.header = file->address;
    if (FONT.header->magic[0] != 0x36 || FONT.header->magic[1] != 0x04) {
        hcf();
    }
    FONT.buffer = (void*)((u64)file->address + sizeof(PSF1Header));
}
