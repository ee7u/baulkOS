#pragma once

#include "types.h"

u64 create_gdt_descriptor(uint32_t base, uint32_t limit, uint16_t flag);

void create_tss_descriptor(u64* gdt, u64 base, u64 limit, u16 flag);

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

void load_gdt();
