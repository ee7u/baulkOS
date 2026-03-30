#include "string.h"

String8 str8(u8* str, u64 size) {
    String8 ret = {str, size};
    return ret;
}

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
