#pragma once

#include "types.h"
#include <stdbool.h>

typedef struct String8 {
    u8* str;
    u64 size;
} String8;

String8 str8(u8* str, u64 size);

#define str8_lit(S) str8((u8*)(S), sizeof(S) - 1)

String8 u64_to_str8(u64 x, u8* buffer, u64 bufferSize);

String8 u64_to_str8_hex(u64 x, u8* buffer, u64 bufferSize);

String8 i64_to_str8(i64 x, u8* buffer, u64 bufferSize);

bool str8_eq(String8 a, String8 b);

String8 str8_suffix(String8 str, u64 n);

String8 str8_from_cstr(const char* cstr);
