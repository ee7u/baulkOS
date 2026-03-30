#pragma once

#include "types.h"

typedef struct InterruptDescriptor {
    u16 offset1;
    u16 selector;
    u8 ist;
    u8 type_attributes;
    u16 offset2;
    u32 offset3;
    u32 reserved;
} InterruptDescriptor;

#define INTERRUPT_GATE 0x8E
#define TRAP_GATE 0x8F

struct interrupt_frame;

#define PIC1 0x20 // address for master PIC
#define PIC2 0x28 // address for slave PIC
#define PIC2_DEFAULT 0xA0 // slave PIC address before remapping
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI	0x20 // End-of-interrupt command code

void PIC_sendEOI(u8 irq);

#define ICW1_ICW4 0x01 // Indicates that ICW4 will be present
#define ICW1_INIT 0x10
#define ICW4_8086 0x01 // 8086/88  mode
#define CASCADE_IRQ 2

void PIC_remap(i32 offset1, i32 offset2);

void IRQ_set_mask(u8 line);

void IRQ_clear_mask(u8 line);

#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b

u16 pic_get_irq_reg(i32 ocw3);

u16 pic_get_irr(void);

u16 pic_get_isr(void);

void set_interrupt_descriptor(u8 i, u64 handler_addr);

void init_idt();

void set_PIT_frequency();

void init_PIC();

__attribute__((interrupt)) void default_interrupt_handler(struct interrupt_frame* frame);

__attribute__((interrupt)) void timer_interrupt_handler(struct interrupt_frame* frame);

__attribute__((interrupt)) void kb_interrupt_handler(struct interrupt_frame* frame);

void init_interrupts();

void sleep(u64 millis);
