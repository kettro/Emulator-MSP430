// Author: Ross Kettleson
// File: lookup_tables.h
// Purpose: Contain all lookup-tables being used in the code

#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "enums.h"

MathShiftLogic_e MathShiftLogic_lt[16] = { // opcode_type, opcode
  // Type 2 opcodes start at 0x04 with MOV, which is not in the ALU
  UNDEFINED_msl,
  UNDEFINED_msl,
  UNDEFINED_msl,
  UNDEFINED_msl,
  UNDEFINED_msl,
  MATH_msl,
  MATH_msl,
  MATH_msl,
  MATH_msl,
  MATH_msl,
  MATH_msl,
  LOGIC_msl,
  LOGIC_msl,
  LOGIC_msl,
  LOGIC_msl,
  LOGIC_msl
};

int Carry_lt[16] = {
  // Uses a carry? 0 = no, 1 = yes
  0,
  0,
  0,
  0,
  0, // mov
  0, 
  1, // addc
  1, // subc
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

int NegSub_lt[16] = {
  // needs to be negated? 
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1, // subc
  1, // sub
  1, // cmp
  0,
  0,
  1, // BIC
  0,
  0,
  0
};

LogicOperation_e LogicOperation_lt[16]= {
// 0 = UNDEFINED_lo
  0,
  0,
  0,
  0,
  0,// 4
  0,
  0,
  0,
  0, // 8
  0,
  0,
  AND_lo, // BIT
  AND_lo, // BIC
  OR_lo,  // BIS
  XOR_lo, // XOR
  AND_lo  // AND
};


#endif
