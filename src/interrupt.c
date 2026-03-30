#include "interrupt.h"
#include "utils.h"
#include "string.h"
#include "keyboard.h"

static InterruptDescriptor idt[256];

void PIC_sendEOI(u8 irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);

	outb(PIC1_COMMAND,PIC_EOI);
}

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

void set_interrupt_descriptor(u8 i, u64 handler_addr) {
    InterruptDescriptor desc;
    desc.offset1 = (u16)(handler_addr & 0xFFFF);
    desc.selector = 0x08;
    desc.ist = 0;
    desc.type_attributes = INTERRUPT_GATE;
    desc.offset2 = (u16)((handler_addr >> 16) & 0xFFFF);
    desc.offset3 = (u32)(handler_addr >> 32);
    idt[i] = desc;
}

void init_idt() {
    for (u64 i = 0; i < 256; i++) {
        set_interrupt_descriptor(i, (u64)default_interrupt_handler);
    }
}

void set_PIT_frequency() {
    outb(0x43, 0x36); // select PIT channel 0
    u16 divisor = 1193; // 1000Hz
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor>>8);
}

void init_PIC() {
    PIC_remap(PIC1, PIC2);
    for (u64 i = 2; i < 16; i++) {
        IRQ_set_mask(i);
    }

    set_PIT_frequency();
}

__attribute__((interrupt)) void default_interrupt_handler(struct interrupt_frame* frame) {
    // TODO: Interrupt service routine should only call a function with attribute 'no_caller_saved_registers' or be compiled with '-mgeneral-regs-only'
    u16 irr = pic_get_irr();
    u16 isr = pic_get_isr();

    u8 buffer[64];
    print_err(str8_lit("Interrupt\n  irr: "));
    print_err(u64_to_str8_hex(irr, buffer, 64));
    print_err(str8_lit("\n  isr: "));
    print_err(u64_to_str8_hex(isr, buffer, 64));
    print_err(str8_lit("\n"));

    PIC_sendEOI(0);
}

static u64 ticks = 0;
__attribute__((interrupt)) void timer_interrupt_handler(struct interrupt_frame* frame) {
    if (ticks > 0) {
        ticks--;
    }
    PIC_sendEOI(0);
}

__attribute__((interrupt)) void kb_interrupt_handler(struct interrupt_frame* frame) {
    KeyCode code = readKeyCode();

    if (code == keyCodeLShiftDown || code == keyCodeRShiftDown) {
        keyboard_state.shift = true;
    } else if (code == keyCodeLShiftUp || code == keyCodeRShiftUp) {
        keyboard_state.shift = false;
    } else if (code == keyCodeLControlDown || code == keyCodeRControlDown) {
        keyboard_state.control = true;
    } else if (code == keyCodeLControlUp || code == keyCodeRControlUp) {
        keyboard_state.control = false;
    } else if (code == keyCodeLMenuDown || code == keyCodeRMenuDown) {
        keyboard_state.menu = true;
    } else if (code == keyCodeLMenuUp || code == keyCodeRMenuUp) {
        keyboard_state.menu = false;
    }

    if (code < N_SIMPLE_KEYCODES) {
        u8 row = 0;
        if (keyboard_state.shift) {
            row = 1;
        } else if (keyboard_state.menu) {
            row = 2;
        }
        u8 ascii_code = ascii_codes[code + row*N_SIMPLE_KEYCODES];
        if (ascii_code != 0) {
            print(str8(&ascii_code, 1));
        }
    }

    PIC_sendEOI(1);
}


extern void setIdt(u16 size, u64 base);
void init_interrupts() {
    setIdt(256*sizeof(InterruptDescriptor)-1, (u64)idt);
    init_idt();

    set_interrupt_descriptor(PIC1, (u64)timer_interrupt_handler);
    set_interrupt_descriptor(PIC1+1, (u64)kb_interrupt_handler);

    init_PIC();
}

void sleep(u64 millis) {
    ticks = millis;
    while (ticks > 0) {
        asm("hlt");
    }
}
