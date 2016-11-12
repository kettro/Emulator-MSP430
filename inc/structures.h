// Author: Ross Kettleson
// File: structures.h
// Purpose: Catalogue All structs used in the code

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "enums.h"

typedef struct _Src_Dst_{
  uint16_t address;       // Effective Address
  uint16_t value;         // The data of the source/destination
  Register_e reg;         // register included with the Instruction
  AddrMode_e addr_mode;   // Addressing Mode: see the enum for options
  MemReg_e mr;
}srcdst_t;

typedef struct _Record_{
  Opcode_e opcode;
  Opcode_Type_e op_type;
  ByteWord_e bw;
  uint8_t as;
  uint8_t ad;
  AOE_e aoe;
  uint16_t instruction;
  srcdst_t src;
  srcdst_t dst;
}record_t;

typedef union _Status_Reg_{
  struct{
    unsigned c: 1;
    unsigned z: 1;
    unsigned n: 1;
    unsigned gie: 1;
    unsigned coff: 1;
    unsigned ooff: 1;
    unsigned scg0: 1;
    unsigned scg1: 1;
    unsigned v: 1;
    unsigned reserved: 7;
  };
  uint16_t w;
}status_reg_t;

typedef union _Data_{
  struct{ // Address individual bits
    unsigned b0: 1;
    unsigned b1: 1;
    unsigned b2: 1;
    unsigned b3: 1;
    unsigned b4: 1;
    unsigned b5: 1;
    unsigned b6: 1;
    unsigned b7: 1;
    unsigned b8: 1;
    unsigned b9: 1;
    unsigned b10: 1;
    unsigned b11: 1;
    unsigned b12: 1;
    unsigned b13: 1;
    unsigned b14: 1;
    unsigned b15: 1;
  };
  struct{ // address the high/low bytes
    uint8_t bl;
    uint8_t bh;
  };
  uint16_t w; // word
}Data_t;

#endif
