// Author: Ross Kettleson
// File: macros.h
// Purpose: Contain all Macros used in the code

#ifndef MACROS_H
#define MACROS_H

// To find the Sign bit of a Byte
#define BIT8(word)            ( (word >> 7) & 0x0001 )
#define BIT16(word)           ( ( word >> 15) & 0x0001 )

// To Mask a Byte or word
#define BYTEMASK(word)        ( (word & 0xFF) )
#define WORDMASK(word)        ( (word & 0xFFFF) )

// To Mask the MSByte and LSByte of a word
#define LSBYTE(word)          ( (word & 0x00FF) )
#define MSBYTE(word)          ( (word >> 8) & 0x00FF);

#endif
