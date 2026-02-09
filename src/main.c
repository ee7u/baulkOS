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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
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

u64 create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
    u64 descriptor;

    descriptor = limit & 0x000F0000;
    descriptor |= (flag <<  8) & 0x00F0FF00;
    descriptor |= (base >> 16) & 0x000000FF;
    descriptor |=  base & 0xFF000000;

    descriptor <<= 32;

    descriptor |= base  << 16;
    descriptor |= limit  & 0x0000FFFF;

    return descriptor;
}

void create_tss_descriptor(u64* gdt, u64 base, u64 limit, u16 flag) {
    gdt[0] = limit & 0x00F0000;
    gdt[0] |= (flag << 8) & 0x00F0FF00;
    gdt[0] |= (base >> 16) & 0x000000FF;
    gdt[0] |= base & 0xFF000000;

    gdt[0] <<= 32;

    gdt[0] |= (base << 16) & 0xFFFF0000;
    gdt[0] |= limit & 0x0000FFFF;

    gdt[1] = base >> 32;
}

extern void setGdt(u16 limit, u64 base);
extern void reloadSegments();
extern void setTSS();
extern void setIdt(u16 size, u64 base);

// from https://forum.osdev.org/viewtopic.php?t=13678
// there seems to be differing terminology for the fields here
typedef volatile struct TSS {
    u16 link;
    u16 link_h;

    u32 esp0;
    u16 ss0;
    u16 ss0_h;

    u32 esp1;
    u16 ss1;
    u16 ss1_h;

    u32 esp2;
    u16 ss2;
    u16 ss2_h;

    u32 cr3;
    u32 eip;
    u32 eflags;

    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;

    u32 esp;
    u32 ebp;

    u32 esi;
    u32 edi;

    u16 es;
    u16 es_h;

    u16 cs;
    u16 cs_h;

    u16 ss;
    u16 ss_h;

    u16 ds;
    u16 ds_h;

    u16 fs;
    u16 fs_h;

    u16 gs;
    u16 gs_h;

    u16 ldt;
    u16 ldt_h;

    u16 trap;
    u16 iomap;
} TSS;

static u64 gdt[7];

typedef struct InterruptDescriptor {
    u16 offset1;
    u16 selector;
    u8 ist;
    u8 type_attributes;
    u16 offset2;
    u32 offset3;
    u32 reserved;
} InterruptDescriptor;

static InterruptDescriptor idt[256];

#define INTERRUPT_GATE 0x8E
#define TRAP_GATE 0x8F

struct interrupt_frame;

void load_gdt() {
    // need to disable interrupts when setting gdt
    asm("cli");

    // TODO: where should we put the gdt?
    gdt[0] = create_descriptor(0, 0, 0);
    gdt[1] = create_descriptor(0, 0x000FFFFF, 0xA09A);
    gdt[2] = create_descriptor(0, 0x000FFFFF, 0xC092);
    gdt[3] = create_descriptor(0, 0x000FFFFF, 0xC0F2);
    gdt[4] = create_descriptor(0, 0x000FFFFF, 0xA0FA);

    // TODO: what should esp0 be? This will be the value of the stack pointer when switching to ring0
    TSS tss = {.ss0 = 0x10, .esp0 = 0, .iomap = sizeof(TSS)};
    create_tss_descriptor(gdt + 5, (u64)&tss, sizeof(tss)-1, 0x4089);

    setGdt(7*8, (u64)gdt);
    reloadSegments();
    asm("sti");
    setTSS();
}

static InterruptDescriptor default_desc;
static InterruptDescriptor timer_desc;
static InterruptDescriptor kb_desc;

static inline void outb(u16 port, u8 val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline u8 inb(u16 port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

#define PIC1 0x20 // address for master PIC
#define PIC2 0x28 // address for slave PIC
#define PIC2_DEFAULT 0xA0 // slave PIC address before remapping
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI	0x20 // End-of-interrupt command code

void PIC_sendEOI(u8 irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);

	outb(PIC1_COMMAND,PIC_EOI);
}

#define ICW1_ICW4 0x01 // Indicates that ICW4 will be present
#define ICW1_INIT 0x10
#define ICW4_8086 0x01 // 8086/88  mode
#define CASCADE_IRQ 2

void PIC_remap(i32 offset1, i32 offset2)
{
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_DEFAULT, ICW1_INIT | ICW1_ICW4);
	outb(PIC1_DATA, offset1);
	outb(PIC2_DEFAULT+1, offset2);
	outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
	outb(PIC1_DATA, ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(PIC2_DATA, ICW4_8086);

	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}

void IRQ_set_mask(u8 line) {
    u16 port;
    u8 value;

    if (line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        line -= 8;
    }
    value = inb(port) | (1 << line);
    outb(port, value);
}

void IRQ_clear_mask(u8 line) {
    u16 port;
    u8 value;

    if (line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        line -= 8;
    }
    value = inb(port) & ~(1 << line);
    outb(port, value);
}

#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b

u16 pic_get_irq_reg(i32 ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

u16 pic_get_irr(void)
{
    return pic_get_irq_reg(PIC_READ_IRR);
}

u16 pic_get_isr(void)
{
    return pic_get_irq_reg(PIC_READ_ISR);
}

u8 print_buffer[128];
__attribute__((interrupt)) void interrupt_handler(struct interrupt_frame* frame) {
    // TODO: Interrupt service routine should only call a function with attribute 'no_caller_saved_registers' or be compiled with '-mgeneral-regs-only'
    u16 irr = pic_get_irr();
    u16 isr = pic_get_isr();

    print_err(str8_lit("Interrupt\n  irr: "));
    print_err(u64_to_str8_hex(irr, print_buffer, 128));
    print_err(str8_lit("\n  isr: "));
    print_err(u64_to_str8_hex(isr, print_buffer, 128));
    print_err(str8_lit("\n"));

    PIC_sendEOI(0);
}

u64 ticks = 0;
__attribute__((interrupt)) void timer_interrupt_handler(struct interrupt_frame* frame) {
    if (ticks > 0) {
        ticks--;
    }
    PIC_sendEOI(0);
}

__attribute__((interrupt)) void kb_interrupt_handler(struct interrupt_frame* frame) {
    u8 code = inb(0x60);
    print_err(str8_lit("Keyboard: "));
    print_err(u64_to_str8_hex(code, print_buffer, 128));
    print_err(str8_lit("\n"));
    PIC_sendEOI(1);
}

void set_PIT_frequency() {
    outb(0x43, 0x36); // select PIT channel 0
    u16 divisor = 1193; // 1000Hz
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor>>8);
}

void sleep(u64 millis) {
    ticks = millis;
    while (ticks > 0) {
        asm("hlt");
    }
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

    if (memmap_request.response == NULL
        || memmap_request.response->entry_count < 1) {
            hcf();
    }

    u64 memmap_entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry** memmap_entries = memmap_request.response->entries;

    for (u64 i = 0; i < memmap_entry_count; i++) {
        print(str8_lit("memmap entry: base="));
        print(u64_to_str8_hex(memmap_entries[i]->base, print_buffer, 128));
        print(str8_lit(" length="));
        print(u64_to_str8_hex(memmap_entries[i]->length, print_buffer, 128));
        print(str8_lit(" type="));
        print(u64_to_str8_hex(memmap_entries[i]->type, print_buffer, 128));
        print(str8_lit("\n"));
    }

    load_gdt();

    setIdt(256*sizeof(InterruptDescriptor)-1, (u64)idt);

    u64 handler_addr = (u64)interrupt_handler;
    default_desc.offset1 = (u16)(handler_addr & 0xFFFF);
    default_desc.selector = 0x08;
    default_desc.ist = 0;
    default_desc.type_attributes = TRAP_GATE;
    default_desc.offset2 = (u16)((handler_addr >> 16) & 0xFFFF);
    default_desc.offset3 = (u32)(handler_addr >> 32);
    for (u64 i = 0; i < 256; i++) {
        idt[i] = default_desc;
    }

    u64 timer_handler_addr = (u64)timer_interrupt_handler;
    timer_desc.offset1 = (u16)(timer_handler_addr & 0xFFFF);
    timer_desc.selector = 0x08;
    timer_desc.ist = 0;
    timer_desc.type_attributes = INTERRUPT_GATE;
    timer_desc.offset2 = (u16)((timer_handler_addr >> 16) & 0xFFFF);
    timer_desc.offset3 = (u32)(timer_handler_addr >> 32);
    idt[PIC1] = timer_desc;

    u64 kb_handler_addr = (u64)kb_interrupt_handler;
    kb_desc.offset1 = (u16)(kb_handler_addr & 0xFFFF);
    kb_desc.selector = 0x08;
    kb_desc.ist = 0;
    kb_desc.type_attributes = INTERRUPT_GATE;
    kb_desc.offset2 = (u16)((kb_handler_addr >> 16) & 0xFFFF);
    kb_desc.offset3 = (u32)(kb_handler_addr >> 32);
    idt[PIC1+1] = kb_desc;

    PIC_remap(PIC1, PIC2);
    for (u64 i = 2; i < 16; i++) {
        IRQ_set_mask(i);
    }

    set_PIT_frequency();

    hcf();
}
