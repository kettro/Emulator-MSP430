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
//uint16_t reg_file[16]; // 16 registers
uint16_t reg_file[16]; // 16 Registers
cache_t cache_line[8][4]; // 32 spot cache; 8 lines of 4

// Local Function Prototypes
void mem(ReadWrite_e rw, ByteWord_e bw);
void reg(Register_e reg_index, ReadWrite_e rw);
void fetch(void);
void updateLRU(cache_t new_line, cache_t old_line);
// External Variables
extern uint16_t MDB_x;
extern uint16_t MAB_x;
extern uint16_t A_x;
extern uint16_t B_x;
extern int debug_flag;
// External Function Prototypes
extern int alu(record_t record, status_reg_t* sr);

// Definitions

//void mem(ReadWrite_e rw, ByteWord_e bw)
//{
//  Data_t data;
//  data.w = MDB_x;
//  uint16_t address = MAB_x;
//
//  if(rw == READ_rw){ // read mode
//    data.bl = memory[address];
//    if(bw == BYTE_bw){ // is a byte
//      data.bh = 0x00;
//    }else{ // word
//      data.bh = memory[address + 1];
//    }
//    MDB_x = data.w; // put the data on the MDB
//  }else{ // write mode
//    memory[address] = data.bl; // assign the low bit to the low memloc
//    if(bw == WORD_bw){ // is byte
//      memory[address + 1] = data.bh;
//    }
//  }
//}
void mem(ReadWrite_e rw, ByteWord_e bw)
{
  int i;
  cache_t new_line = {
    .w = MDB_x,
    .address = MAB_x,
    .lru = 0
  };
  cache_t found_line;
  cache_t oldest_line = { .lru = 0 };
  new_line.block = new_line.address % 8;
  if(rw == READ_rw){ MDB_x = 0x0000; }

  for(i = 0; i < 4; i++){
    found_line= cache_line[new_line.block][i];
    if(found_line.address == new_line.address){
      // Hit
      if(rw == READ_rw){
        MDB_x = (bw == WORD_bw) ? found_line.w : found_line.bl;
      }else{
        new_line.db = 1;
      }
      break;
    }else{
      // Miss
      if(found_line.lru > oldest_line.lru){
        oldest_line = found_line; // keeping track of the oldest line in the block;
      }
    }
  }

  if(i == 4){
    // No hit- looped all the way through
    found_line = oldest_line;
    if(found_line.db == 1){
      // Need to write through
      memory[found_line.address] = found_line.bl;
      if(bw == WORD_bw){ 
        memory[found_line.address + 1] = found_line.bh;
      }
    }
    // Need to get the data of the new_line into the new_line
    if(rw == READ_rw){
      new_line.bl = memory[new_line.address];
      if(bw == WORD_bw){
        new_line.bh = memory[new_line.address + 1];
        MDB_x = new_line.w;
      }else{
        MDB_x = new_line.bl;
      }
    }else{
      // Write
      // Not writing to the memory location, setting db so it does it for me, later
      new_line.db = 1;
    }
  }

  updateLRU(new_line, found_line);
  return;
}

void updateLRU(cache_t new_line, cache_t old_line)
{
  int block = new_line.block;
  int i;
  cache_t* line;
  for(i = 0; i < 4; i++){
    line = &cache_line[block][i];
    if(line->lru < old_line.lru){
      line->lru++;
    }else if(line->lru == old_line.lru){
      // if it is the same age as the old block
      *line = new_line; // Drop in the new_line, regardless if write or read- updated!
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
  if(debug_flag){ printf("Fetching @ %x\n", MAB_x); }
  A_x = MDB_x;
  B_x = 0x0002; // from Constant Generator
  record_t inc_rec = { .op_type = TWO_opt, .opcode = ADD_op, .bw = WORD_bw}; // dummy record to provide add opcode
  alu(inc_rec, NULL); // Increment PC, but don't update the SR
  reg(PC, WRITE_rw); // write to the PC
  // Get the desired mem location
  mem(READ_rw, WORD_bw); // read the previous value of the PC from memory
  if(debug_flag){ printf("Fetched %x\n", MDB_x); }
}
