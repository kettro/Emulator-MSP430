// Author: Ross Kettleson
// File: lookup_tables.h
// Purpose: Contain all lookup-tables being used in the code

#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "enums.h"

static MathShiftLogic_e MathShiftLogic_lt[16] = { // opcode_type, opcode
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

static int Carry_lt[16] = {
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

static int NegSub_lt[16] = {
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

static LogicOperation_e LogicOperation_lt[16]= {
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

static char ConstantGeneratorAsReg[4][4] = {
  // only from CG1 and CG2 = R3, R2
  // if -2: error: not possible
  {-2,-2,-2,0}, // As = 0
  {-2,-2,-2,1}, // As = 1
  {-2,-2,4,2}, // As = 2
  {-2,-2,8,-1}  // As = 3
};

static AddrMode_e GetAddrMode_lt[4][16] = {
  // 0 = REG_DIR
  // 1 = indexed, relative, or absolute
  // 2 = indirect
  // 3 = indirect_aa
  // 4 = immediate; Immediate iff Using SR/R3, with As: otherwise, it is the std value
  {REG_DIR,REG_DIR,REG_DIR,IMM_CG,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR,REG_DIR},
  {RELATIVE,INDEXED,ABSOLUTE,IMM_CG,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED,INDEXED},
  {INDIRECT,INDIRECT,IMM_CG,IMM_CG,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT,INDIRECT},
  {IMMEDIATE,INDIRECT_AA,IMM_CG,IMM_CG,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA,INDIRECT_AA}
};


#endif
