// Author : Ross Kettleson
// File: execute.c
// Purpose: Process the execution of Instructions, switched on
//          Area of Execution. E step of FDE

// Include Files
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "structures.h"
#include "macros.h"
#include "lookup_tables.h"

// Defines and Macros
// Local Variables
// Local Function Prototypes
void execute(record_t record); // PUBLIC
void executeALU(record_t record);
void executeID(record_t record);
void executeSP(record_t record);

// External Variables
extern uint16_t MDB_x;
extern uint16_t MAB_x;
extern uint16_t A_x;
extern uint16_t B_x;
// External Function Prototypes
extern int alu(record_t record, status_reg_t* sr); // if ret = 0: no value on MDB
extern void mem(ReadWrite_e rw, ByteWord_e bw);
extern void reg(Register_e reg_index, ReadWrite_e rw);

// Definitions

void execute(record_t record)
{
  // the AOE determines how a Function is executed
  switch(record.aoe){
    case ID_aoe:
      // This is, now (JUL11) only taken by MOV
      executeID(record);
      break;
    case SP_aoe:
      // this is the 1op insts that use the stack- reti, call, push
      executeSP(record);
      break;
    case ALU_aoe:
      // Everything else, really;
      // the 1ops are handled using "external hardware", inside the alu
      // Others are seperated by MATH and LOGIC
      // MATH is boiled down to being just addition (outside DADD)
      // LOGIC is AND, OR, or XOR (I don't want to have XOR be 3 different calls)
      executeALU(record);
      break;
  }
}

void executeALU(record_t record)
{
  // Amounts to calling alu() on the record, with the SR
  A_x = record.src.value;
  B_x = record.dst.value;
  status_reg_t sr;
  reg(SR, READ_rw); // get SR from reg file
  sr.w = MDB_x;
  if(alu(record, &sr)){ // if it returns a value, put it in the right place
    if(record.dst.mr == MEM_mr){
      MAB_x = record.dst.address;
      mem(WRITE_rw, WORD_bw);
    }else{
      reg(record.dst.reg, WRITE_rw);
    }
  }
}

void executeID(record_t record)
{
  if(!((record.op_type == TWO_opt) && (record.opcode == MOV_op))){
    // error: this AOE is only used by MOV!
    return;
  }
  MDB_x = record.src.value;
  if(record.dst.mr == MEM_mr){
    MAB_x = record.dst.address;
    mem(WRITE_rw, record.bw);
  }else{
    reg(record.dst.reg, WRITE_rw);
  }
}

void executeSP(record_t record)
{
  record_t sub_rec = {
    .op_type = TWO_opt,
    .opcode = SUB_op,
    .bw = WORD_bw
  };
  if(record.op_type == ONE_opt && record.opcode == RETI_op){
    // is RETI
    reg(SP, READ_rw); // put SP on MDB
    MAB_x = MDB_x;
    mem(READ_rw, WORD_bw); // read from mem[SP]
    reg(SR, WRITE_rw); // write to the SR
    reg(SP, READ_rw); // get the stack pointer on the MDB
    A_x = 2;
    B_x = MDB_x;
    alu(sub_rec, NULL); // SP-2 -> MDB
    MAB_x = MDB_x;
    mem(READ_rw, WORD_bw);
    reg(PC, WRITE_rw); // write mem[SP] to the PC
  }else{ // not RETI
    reg(SP, READ_rw); // put the SP on the MDB
    A_x = 2;
    B_x = MDB_x; // SP on the ALU B
    alu(sub_rec, NULL);
    MAB_x = MDB_x; // SP-2 -> MAB
    switch(record.opcode){
      case CALL_op:
        reg(PC, READ_rw); // puts PC on the MDB
        mem(WRITE_rw, WORD_bw); // SP-2 on MAB, PC on MDB
        MDB_x = record.dst.value;
        reg(PC, WRITE_rw); // write the destination to the PC, to call the fn
        break;
      case PUSH_op:
        MDB_x = record.src.value; // value on the MDB
        mem(WRITE_rw, record.bw);
        break;
      default:
        //error: shouldn't have anything else
        break;
    }// end switch
  } // end not reti
}
