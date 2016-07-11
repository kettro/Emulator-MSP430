// Author : Ross Kettleson
// File: mem_reg.c
// Purpose: Manage the Memory and the Registers

// Include Files
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "enums.h"
#include "structures.h"
// Defines and Macros
// Local Variables
uint16_t memory[0x10000]; // 2^16 memory locations
uint16_t reg_file[16]; // 16 registers

// Local Function Prototypes
void mem(ReadWrite_e rw, ByteWord_e bw);
void reg(Register_e reg_index, ReadWrite_e rw);
void fetch(void);
// External Variables
extern uint16_t MDB_x;
extern uint16_t MAB_x;
extern uint16_t A_x;
extern uint16_t B_x;
// External Function Prototypes
extern int alu(record_t record, status_reg_t* sr);

// Definitions

void mem(ReadWrite_e rw, ByteWord_e bw)
{
  Data_t data;
  data.w = MDB_x;
  uint16_t address = MAB_x;
  if(rw == READ_rw){ // read mode
    data.bl = memory[address];
    if(bw == BYTE_bw){ // is a byte
      data.bh = 0x00;
    }else{ // word
      data.bh = memory[address + 1];
    }
    MDB_x = data.w; // put the data on the MDB
  }else{ // write mode
    memory[address] = data.bl; // assign the low bit to the low memloc
    if(bw == WORD_bw){ // is byte
      memory[address + 1] = data.bh;
    }
  }
}

void reg(Register_e reg_index, ReadWrite_e rw)
{
  uint16_t* reg = &reg_file[reg_index];
  switch(rw){
    case READ_rw:
      MDB_x = *reg; // put the value of the register on the MDB
      break;
    case WRITE_rw:
      *reg = MDB_x; // get the value from the MDB
      break;
  }
}

void fetch(void)
{
  reg(PC, READ_rw); // put PC on the MDB
  MAB_x = MDB_x; // Put the PC on the MAB, not changed in the following:
  // Increment the PC
  A_x = MDB_x;
  B_x = 0x0002; // from Constant Generator
  record_t inc_rec = { .op_type = TWO_opt, .opcode = ADD_op, .bw = WORD_bw}; // dummy record to provide add opcode
  alu(inc_rec, NULL); // Increment PC, but don't update the SR
  reg(PC, WRITE_rw); // write to the PC
  // Get the desired mem location
  mem(READ_rw, WORD_bw); // read the previous value of the PC from memory
}
