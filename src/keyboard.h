#pragma once

#include "types.h"
#include "utils.h"

// TODO: This is a super hacky quick way to just get this working, not an attempt at a good keyboard driver :)
#define N_SIMPLE_KEYCODES 0x3A
u8 ascii_codes[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', 0, 0, '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0, 0, 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0, 0, 0, 0, 0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, '*', 0, ' ',
                      0, 0, '!', '"', '#',   0, '%', '&', '/', '(', ')', '=', '?', 0, 0,    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0, 0, 0, 0, 0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0,   0, 0,   0,
                      0, 0,   0, '@',   0,   0,   0,   0, '{', '[', ']', '}', '\\',0, 0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,   0, 0,   0};

typedef enum KeyCode {
    keyCodeUndefined = 0x00,
    keyCodeEscape = 0x01,
    keyCode1 = 0x02,
    keyCode2 = 0x03,
    keyCode3 = 0x04,
    keyCode4 = 0x05,
    keyCode5 = 0x06,
    keyCode6 = 0x07,
    keyCode7 = 0x08,
    keyCode8 = 0x09,
    keyCode9 = 0x0A,
    keyCode0 = 0x0B,
    keyCodeOEMPlus = 0x0C,
    keyCodeOEM4 = 0x0D,
    keyCodeBack = 0x0E,
    keyCodeTab = 0x0F,
    keyCodeQ = 0x10,
    keyCodeW = 0x11,
    keyCodeE = 0x12,
    keyCodeR = 0x13,
    keyCodeT = 0x14,
    keyCodeY = 0x15,
    keyCodeU = 0x16,
    keyCodeI = 0x17,
    keyCodeO = 0x18,
    keyCodeP = 0x19,
    keyCodeOEM6 = 0x1A,
    keyCodeOEM1 = 0x1B,
    keyCodeReturn = 0x1C,
    keyCodeLControlDown = 0x1D,
    keyCodeA = 0x1E,
    keyCodeS = 0x1F,
    keyCodeD = 0x20,
    keyCodeF = 0x21,
    keyCodeG = 0x22,
    keyCodeH = 0x23,
    keyCodeJ = 0x24,
    keyCodeK = 0x25,
    keyCodeL = 0x26,
    keyCodeOEM3 = 0x27,
    keyCodeOEM7 = 0x28,
    keyCodeOEM5 = 0x29,
    keyCodeLShiftDown = 0x2A,
    keyCodeOEM2 = 0x2B,
    keyCodeZ = 0x2C,
    keyCodeX = 0x2D,
    keyCodeC = 0x2E,
    keyCodeV = 0x2F,
    keyCodeB = 0x30,
    keyCodeN = 0x31,
    keyCodeM = 0x32,
    keyCodeComma = 0x33,
    keyCodePeriod = 0x34,
    keyCodeMinus = 0x35,
    keyCodeRShiftDown = 0x36,
    keyCodeMultiply = 0x37,
    keyCodeLMenuDown = 0x38,
    keyCodeSpace = 0x39,
    keyCodeCapital = 0x3A,
    keyCodeF1 = 0x3B,
    keyCodeF2 = 0x3C,
    keyCodeF3 = 0x3D,
    keyCodeF4 = 0x3E,
    keyCodeF5 = 0x3F,
    keyCodeF6 = 0x40,
    keyCodeF7 = 0x41,
    keyCodeF8 = 0x42,
    keyCodeF9 = 0x43,
    keyCodeF10 = 0x44,
    keyCodeNumlock = 0x45,
    keycodeScroll = 0x46,
    keyCodeHome = 0x47,
    keyCodeSubtract = 0x4A,
    keyCodeClear = 0x4C,
    keyCodeAdd = 0x4E,
    keyCodeSnapshot = 0x55,
    keyCodeOEM102 = 0x56,
    keyCodeF11 = 0x57,
    keyCodeF12 = 0x58,
    keyCodeLControlUp = 0x9D,
    keyCodeLShiftUp = 0xAA,
    keyCodeRShiftUp = 0xB6,
    keyCodeLMenuUp = 0xB8,
    keyCodeRControlDown = 0xE01D,
    keyCodeDivide = 0xE035,
    keyCodeRMenuDown = 0xE038,
    keyCodeUp = 0xE048,
    keyCodePrior = 0xE049,
    keyCodeLeft = 0xE04B,
    keyCodeRight = 0xE04D,
    keyCodeEnd = 0xE04F,
    keyCodeDown = 0xE050,
    keyCodeNext = 0xE051,
    keyCodeInsert = 0xE052,
    keyCodeDelete = 0xE053,
    keyCodeLWin = 0xE05B,
    keyCodeRWin = 0xE05C,
    keyCodeFN = 0xE05D,
    keyCodeRControlUp = 0xE09D,
    keyCodeRMenuUp = 0xE0B8,
    keyCodePause = 0xE11D
} KeyCode;

typedef struct KeyboardState {
   bool shift;
   bool control;
   bool menu;
} KeyboardState;

KeyboardState keyboard_state;

KeyCode readKeyCode() {
    u8 byte1 = inb(0x60);
    if (byte1 != 0xE0) {
        return (KeyCode)byte1;
    } else {
        u8 byte2 = inb(0x60);
        return (KeyCode)((byte1 << 8) | byte2);
    }
}
