// Author: Ross Kettleson
// File: enums.h
// Purpose: Contain all enumerated types used in the code

#ifndef ENUMS_H
#define ENUMS_H

typedef enum _Register_List_{
  R0 = 0,
  PC = 0,
  R1 = 1,
  SP = 1,
  R2 = 2,
  SR = 2,
  CG1 = 2,
  R3 = 3,
  CG2 = 3,
  R4 = 4,
  R5 = 5,
  R6 = 6,
  R7 = 7,
  R8 = 8,
  R9 = 9,
  R10 = 10,
  R11 = 11,
  R12 = 12,
  R13 = 13,
  R14 = 14,
  R15 = 15
} Register_e;

typedef enum _Area_of_Execution_{
  ID_aoe,
  SP_aoe,
  ALU_aoe
}AOE_e;

typedef enum _ReadWrite_{
  READ_rw = 0,
  WRITE_rw = 1
}ReadWrite_e;

typedef enum _ByteWord_{
  BYTE_bw = 0,
  WORD_bw = 1
}ByteWord_e;

typedef enum _Opcode_Type_{
  ONE_opt = 0,
  JUMP_opt = 1,
  TWO_opt = 2,
}Opcode_Type_e;

typedef enum _OpCodes_{
  // identify the instruction using the opcode and the type
  // type 1:
  RRC_op = 0,
  SWPB_op = 1,
  RRA_op = 2,
  SXT_op = 3,
  PUSH_op = 4,
  CALL_op = 5,
  RETI_op = 6,
  // Jumps
  JNE_op = 0,
  JEQ_op = 1,
  JNC_op = 2,
  JC_op = 3,
  JN_op = 4,
  JGE_op = 5,
  JL_op = 6,
  JMP_op = 7,
  // Type 2
  MOV_op = 4,
  ADD_op = 5,
  ADDC_op = 6,
  SUBC_op = 7,
  SUB_op = 8,
  CMP_op = 9,
  DADD_op = 10,
  BIT_op = 11,
  BIC_op = 12,
  BIS_op = 13,
  XOR_op = 14,
  AND_op = 15
}Opcode_e;

typedef enum _Addr_Mode_{
  REG_DIR = 0,
  INDEXED,
  RELATIVE,
  ABSOLUTE,
  INDIRECT,
  INDIRECT_AA,
  IMMEDIATE,
  IMM_CG
}AddrMode_e;

typedef enum _Math_Shift_Logic_{
  MATH_msl = 0,
  SHIFT_msl = 1,
  LOGIC_msl = 2,
  UNDEFINED_msl = 3,
}MathShiftLogic_e;

typedef enum _Logic_Operations_{
  UNDEFINED_lo = 0,
  AND_lo,
  OR_lo,
  XOR_lo,
}LogicOperation_e;

typedef enum _MemRegCG_{
  MEM_mrc,
  REG_mrc,
  CG_mrc
}MemRegCG_e;

typedef enum _SrcDst_{
  SRC_sd,
  DST_sd
}SrcDst_e;

#endif
