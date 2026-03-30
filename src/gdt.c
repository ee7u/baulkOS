#include "gdt.h"

u64 create_gdt_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
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

static u64 gdt[7];

void load_gdt() {
    // need to disable interrupts when setting gdt
    asm("cli");

    // TODO: where should we put the gdt?
    gdt[0] = create_gdt_descriptor(0, 0, 0);
    gdt[1] = create_gdt_descriptor(0, 0x000FFFFF, 0xA09A);
    gdt[2] = create_gdt_descriptor(0, 0x000FFFFF, 0xC092);
    gdt[3] = create_gdt_descriptor(0, 0x000FFFFF, 0xC0F2);
    gdt[4] = create_gdt_descriptor(0, 0x000FFFFF, 0xA0FA);

    // TODO: what should esp0 be? This will be the value of the stack pointer when switching to ring0
    TSS tss = {.ss0 = 0x10, .esp0 = 0, .iomap = sizeof(TSS)};
    create_tss_descriptor(gdt + 5, (u64)&tss, sizeof(tss)-1, 0x4089);

    setGdt(7*8, (u64)gdt);
    reloadSegments();
    asm("sti");
    setTSS();
}
