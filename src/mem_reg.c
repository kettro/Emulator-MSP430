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
#define BLOCKS  4
#define SLOTS   4
// Local Variables
uint16_t memory[0x10000]; // 2^16 memory locations
//uint16_t reg_file[16]; // 16 registers
uint16_t reg_file[16]; // 16 Registers
cache_t cache_line[BLOCKS][SLOTS]; // 32 spot cache; 8 lines of 4

// Local Function Prototypes
void mem(ReadWrite_e rw, ByteWord_e bw);
void reg(Register_e reg_index, ReadWrite_e rw);
void fetch(void);
void lru(cache_t new_line, int target_lru);
// External Variables
extern uint16_t MDB_x;
extern uint16_t MAB_x;
extern uint16_t A_x;
extern uint16_t B_x;
extern int debug_flag;
extern int cache_dump;
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


/* Function: mem
 * Description: Accesses the memory, as well as the cache
 * Parameters: Read/Write, and Byte/Word designation
 * Return: none
 */
void mem(ReadWrite_e rw, ByteWord_e bw)
{
  if(debug_flag && cache_dump){ printf("Querying Memory @%X: ", MAB_x); }
  int i;
  cache_t new_line = {
    .w = 0,
    .address = MAB_x,
    .lru = 0,
    .block = (MAB_x/2) % BLOCKS // in order to make the cache run on words, not bytes
  };
  cache_t found_line;
  int desired_lru;
  new_line.bl |= MDB_x & 0xFF;
  if(bw == WORD_bw){ new_line.bh = ((MDB_x >> 8) & 0xFF); }
  //cache_t oldest_line = { .lru = 0 };
  //new_line.block = new_line.address % 8;
  if(rw == READ_rw){ MDB_x = 0x0000; }
  // Search for a hit
  for(i = 0; i < SLOTS; i++){
    found_line = cache_line[new_line.block][i];
    if(found_line.address == new_line.address){
      // Hit
      if(debug_flag & cache_dump){ printf("Hit\n"); }
      if(rw == READ_rw){
        new_line.w = (bw == WORD_bw) ? found_line.w : found_line.bl;
        MDB_x = new_line.w;
      }else{
        new_line.db = 1;
      }
      desired_lru = found_line.lru;
      lru(new_line, desired_lru);
      return;
    }
  }
  // Miss
  if(debug_flag && cache_dump){ printf("Miss\n"); }
  desired_lru = 3; // replace the eldest
  // Need to find the correct record, and write through
  for(i = 0; i < SLOTS; i++){
    found_line = cache_line[new_line.block][i];
    if(found_line.lru == desired_lru){
      break; // Gottem
    }
  }

  if(found_line.db == 1){
    // Need to write through
    if(debug_flag && cache_dump){ printf("Writing through @%X\n", found_line.address); }
    memory[found_line.address] = found_line.bl;
    if(bw == WORD_bw){
      memory[found_line.address + 1] = found_line.bh;
    }
  }
  // Need to the get the data of the new_line into the new_line
  if(rw == READ_rw){
    new_line.bl = memory[new_line.address];
    if(bw == WORD_bw){
      new_line.bh = memory[new_line.address + 1];
      MDB_x = new_line.w;
    }else{
      MDB_x = new_line.bl;
    }
  }else{
    // write
    // Not writing to the memory location here, setting the db, so it does it for me
    new_line.db = 1;
  }
  lru(new_line, desired_lru);
  return;
}

/* Function: lru
 * Description: For the cache, determines the least recently used slot, and replaces it
 * Parameters: the cache entry to be inserted, the lru of the item to be replaced
 * Return: none
 */
void lru(cache_t new_line, int target_lru)
{
  int i;
  cache_t* line;
  for(i = 0; i < SLOTS; i++){
    line = &cache_line[new_line.block][i];
    if(line->lru == target_lru){
      // Bingo
      *line = new_line;
      line->lru = 0;
    }else if(line->lru < target_lru){
      line->lru++;
    }
  }
}

/* Function: reg
 * Description: accesses the register file
 * Parameters: Index to the register, Read/Write designation
 * Return: none
 */
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

/* Function: fetch
 * Description: Fetches an item from memory, and then increments the PC
 * Parameters: none
 * Return: none
 */
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
