#include "utils.h"
#include <stdint.h>
#include <stddef.h>

#include "types.h"

void *memcpy(void *restrict dest, const void *restrict src, u64 n) {
    u8 *restrict pdest = (u8 *restrict)dest;
    const u8 *restrict psrc = (const u8 *restrict)src;

    for (u64 i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, i32 c, u64 n) {
    u8 *p = (u8 *)s;

    for (u64 i = 0; i < n; i++) {
        p[i] = (u8)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, u64 n) {
    u8 *pdest = (u8 *)dest;
    const u8 *psrc = (const u8 *)src;

    if (src > dest) {
        for (u64 i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (u64 i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

i32 memcmp(const void *s1, const void *s2, u64 n) {
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    for (u64 i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

void outb(u16 port, u8 val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}
