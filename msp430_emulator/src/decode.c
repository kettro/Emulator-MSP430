// Author : Ross Kettleson
// File: decode.c
// Purpose: Contains functions to interpret the incoming instructions

// Include Files
#include <stdint.h>
#include <stdlib.h>
#include "structures.h"
#include "lookup_tables.h"

// Defines and Macros
#define INDIR_AA_INCREMENT    1
// Local Variables
// Local Function Prototypes
void parseOpcode(record_t* record);
void parseOperand(record_t* record);
void calculateOffset(record_t* record, status_reg_t* sr);
void calculateOperandAddress(record_t* record, SrcDst_e target);
record_t decode(void);
// External Variables
extern uint16_t A_x;
extern uint16_t B_x;
extern uint16_t MAB_x;
extern uint16_t MDB_x;
// External Function Prototypes
extern int alu(record_t record, status_reg_t* sr);
extern void fetch(void);
extern void mem(ReadWrite_e rw, ByteWord_e bw);
extern void reg(Register_e reg_index, ReadWrite_e rw);

// Definitions

void parseOpcode(record_t* record)
{
  uint16_t type_mask = 0xE000; // >> 13
  uint16_t jump_op_mask = 0x1C00; // >> 10
  uint16_t t1_op_mask = 0x01C0; // >> 7
  uint16_t t2_op_mask = 0xF000; // >> 12

  switch((record->instruction & type_mask) >> 12){
    case 0:
      record->op_type = ONE_opt;
      record->opcode = (record->instruction & t1_op_mask) >> 7;
      break;
    case 1:
      record->op_type = JUMP_opt;
      record->opcode = (record->instruction & jump_op_mask) >> 10;
      break;
    default:
      record->op_type = TWO_opt;
      record->opcode = (record->instruction & t2_op_mask) >> 12;
      break;
  }
  if(record->op_type == ONE_opt){
    switch(record->opcode){
      case PUSH_op:
      case CALL_op:
      case RETI_op:
        record->aoe = SP_aoe;
        break;
      default: // the shifts, swpb, and sxt
        record->aoe = ALU_aoe;
    }
  }
  else if(record->op_type == TWO_opt && record->opcode == MOV_op){ // Mov is *special*
    record->aoe = ID_aoe;
  }
  else{ // all jumps and all other Type 2's
    record->aoe = ALU_aoe;
  }
  return;
}

void parseOperand(record_t* record)
{
  uint16_t jump_mask = 0x01FF; // bits 9->0
  uint16_t b3_0_mask = 0x0007; // bits 3->0
  uint16_t b11_8_mask = 0x0F00; // bits 11->8
  Data_t inst_d;
  status_reg_t sr;
  inst_d.w = record->instruction;
  // return if RETI
  if((record->op_type == ONE_opt) && (record->opcode == RETI_op)){ return; }
  if(record->op_type == JUMP_opt){ // Jumps
    // src is the calculated offset, calculated by calculateOffset()
    // dst is the uncalculated offset
    record->dst.mr = REG_mr;
    record->dst.value = inst_d.w & jump_mask; // op = bits 9->0
    reg(SR, READ_rw); // get the SR onto the MDB
    sr.w = MDB_x;
    calculateOffset(record, &sr); // offset needs to be translated
  }else{ // not jumps
    record->bw = inst_d.b6; // Bit6 = BW
    record->as = (inst_d.b5 << 1) | inst_d.b4;
    if(record->op_type == ONE_opt){
      record->src.reg = b3_0_mask & inst_d.w; // get bits 3->0
    }else{ // type 2
      record->src.reg = (b11_8_mask & inst_d.w) >> 8; // getting the reg
      record->ad = inst_d.b7;
      record->dst.reg = inst_d.w & b3_0_mask; // no right shift needed
      calculateOperandAddress(record, DST_sd);
    }
    // both type 1 and type 2
    calculateOperandAddress(record, SRC_sd);
    if(record->op_type == ONE_opt && record->aoe == ALU_aoe){
      // type one in the alu => need to double up the arguments
      record->dst = record->src;
    }
  }// end not-jumps
  return;
}

void calculateOffset(record_t* record, status_reg_t* sr)
{
  if(sr == NULL){ return; }
  // dynamic lookup_table to check conditions
  int sr_condition_table[8] = {
    (sr->z == 0),
    (sr->z =! 0),
    (sr->c == 0),
    (sr->c =! 0),
    (sr->n == 1),
    ((sr->n ^ sr->v) == 0),
    ((sr->n ^ sr->v) == 1),
    (1)
  };
  record_t add_rec = { .op_type = TWO_opt, .opcode = ADD_op, .bw = WORD_bw}; // dummy record for adding
  if(sr_condition_table[record->opcode]){
    A_x = record->dst.value;
    B_x = record->dst.value; // multiplying the dst by 2
    alu(add_rec, NULL); // TODO: maybe increase sysclock?
    record->src.value = MDB_x; // the offset is now double the passed value
  }else{
    record->src.value = 0x00; // if the condition isn't met, PC = PC + 0
  }
  reg(PC, READ_rw); // get the value of the PC and put it on the MDB
  record->dst.value = MDB_x; // use the PC as the dst
  record->opcode = ADD_op;
  record->op_type = TWO_opt;
  record->bw = WORD_bw;
  // TODO: increase SYCLOCK? 
  return;
}

void calculateOperandAddress(record_t* record, SrcDst_e target_type)
{
  // variable declaration
  int asd; // either the as or the ad
  int cg_val;
  srcdst_t* target;
  record_t add_record = {
    .opcode = ADD_op,
    .op_type = TWO_opt,
    .bw = WORD_bw
  };
  if(target_type == SRC_sd){ 
    asd = record->as; 
    target = &record->src;
  }else{
    asd = record->ad;
    target = &record->dst;
  }
  target->addr_mode = GetAddrMode_lt[asd][target->reg]; // get addr mode
  // Fix DST as an immediate
  if(target->addr_mode == IMM_CG){
    if(target_type == DST_sd){ // dst cannot be cg: underlying!
      target->addr_mode = record->ad ? INDEXED : REG_DIR; // ad=0: reg_dir, ad=1: INDEXED
    }
  }
  // calculate:
  switch(target->addr_mode){
    case REG_DIR:
      target->mr = REG_mr;
      reg(target->reg, READ_rw); // get the value of the register
      target->value = MDB_x; // get the value from the MDB
      target->mr = REG_mr;
      break;
    case IMM_CG:
      // get value from the table
      target->mr = REG_mr;
      target->value = ConstantGeneratorAsReg[record->as][target->reg];
      break;
    case INDIRECT:
    case INDIRECT_AA: 
      target->mr = REG_mr;
      reg(target->reg, READ_rw);
      target->address = MDB_x; // Address from the MDB = the register
      if(target->addr_mode == INDIRECT_AA){
        reg(target->reg, READ_rw);
        A_x = MDB_x;
        B_x = INDIR_AA_INCREMENT; // constant generated
        alu(add_record, NULL); // increment the register by 1
        reg(target->reg, WRITE_rw); // store the result in the register
        break;
      }
      MAB_x = target->address; // put the effective address on the MAB
      mem(READ_rw, WORD_bw); // get the data at that location
      target->value = MDB_x; // get the value of that data from the MDB
      break;
    case IMMEDIATE:
      target->mr = MEM_mr;
      fetch(); // get the next value
      target->value = MDB_x;
      break;
    default:
      // handle memory type
      target->mr = MEM_mr;
      fetch(); // get the value of the next entry in the PC, put on MDB
      switch(target->addr_mode){ // switch in a switch
        case ABSOLUTE:
          MAB_x = MDB_x; // the address is the value from mem[pc]
          break;
        case INDEXED:
          // target->reg = PC;
        case RELATIVE:
          A_x = MDB_x; // mem[pc] + REG = Address; for indexed: reg = PC
          reg(target->reg, READ_rw);
          B_x = MDB_x;
          alu(add_record, NULL);
          break;
      } // end switch in a switch
      mem(READ_rw, WORD_bw);
      target->value = MDB_x;
      break;
  }
  return;
}

record_t decode(void)
{
  record_t record;
  record.instruction = MDB_x;
  parseOpcode(&record);
  parseOperand(&record);
  return record;
}
