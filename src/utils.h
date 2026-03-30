#pragma once

#include <stdint.h>
#include <stddef.h>

#include "types.h"

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
void* memcpy(void* restrict dest, const void* restrict src, u64 n);

void* memset(void* s, i32 c, u64 n);

void* memmove(void* dest, const void* src, u64 n);

i32 memcmp(const void* s1, const void* s2, u64 n);

void outb(u16 port, u8 val);

u8 inb(u16 port);

void hcf(void);
