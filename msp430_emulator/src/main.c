// Author : Ross Kettleson
// File: main.c
// Purpose: provide the main loop of execution, main()

// Include Files
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"
// Defines and Macros
// Local Variables
  // Buses
uint16_t A_x;
uint16_t B_x;
uint16_t MDB_x;
uint16_t MAB_x;

// Local Function Prototypes
void initialSetup(FILE* input_file);
uint16_t FDEI(void);
// External Variables
// External Function Prototypes
extern void fetch(void);
extern record_t decode(void);
extern void execute(record_t record);
extern uint16_t loader(FILE* input_file);
extern void reg(Register_e reg_index, ReadWrite_e rw);

// Definitions

int main(int argc, char** argv)
{
  // main
  FILE* file;
  if(argc < 2){
    printf("Input File Required\n");
    return 0;
  }
  int i;
  for(i = 0; i < argc; i++){
    if(strchr(argv[i], '-')){
      // there is a flag, handle it, and handle the file, I guess
      break;
    }
  }
  if(i == argc){
    // no flags
    file = fopen(argv[1], "r");
  }

  initialSetup(file);
  i = 0;
  while(FDEI() != 0xFFFF){ // while PC != RAMEND
    if(++i > 3) break; // short stop point
  }
  return 0;
}

void initialSetup(FILE* input_file)
{
  // Setting up the initial state of the code.
  // Call the loader, and fill the mem;
  // store the initial mem location of the code
  // so the PC can begin at a define point
  uint16_t start_location = 0x0020; // address directly after the devices
  start_location = loader(input_file);
  // set the start location to the start_location
  MDB_x = start_location;
  reg(PC, WRITE_rw);
  // clear the SR
  MDB_x = 0x0000;
  reg(SR, WRITE_rw);
  // set the SP to RAMEND = 0xFFFF, as default
  MDB_x = 0xFFFF;
  reg(SP, WRITE_rw);
}

uint16_t FDEI(void)
{
  // Fetch, Decode, Execute, Interrupts
  uint16_t pc;
  fetch();
  pc = MAB_x; // the address fetched
  if(pc == 0xFFFF){ 
    return pc; }
  // the new value is on the MDB, and PC+=2
  // process the new instruction, and determine the operands, etc
  // Returns a fully featured record
  record_t record;
  record = decode();
  // Now that we have the id of the inst, its operand(s), and its
  // area of execution: can actually execute that code. 
  // Jumps are now just addition, and everything is either handled
  // by the ALU, external ALU hardware, or by the ID itself (SP/ID)
  execute(record);
  return pc;

}
