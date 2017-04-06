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

int debug_flag = 0;
int reg_dump = 0;
int cache_dump = 0;
// Local Function Prototypes
void initialSetup(FILE* input_file);
uint16_t FDEI(void);
// External Variables
extern uint16_t reg_file[16];
extern cache_t cache_line[8][4];
// External Function Prototypes
extern void fetch(void);
extern record_t decode(void);
extern void execute(record_t record);
extern uint16_t loader(FILE* input_file);
extern void reg(Register_e reg_index, ReadWrite_e rw);

// Definitions

/* Function: main
 * Description: Runs the entire code
 * Parameters: the flags, as well as the input file
 * Return: none
 */
int main(int argc, char** argv)
{
  // main
  FILE* file;
  char* flagptr;

  if(argc < 2){
    printf("Input File Required\n");
    return 0;
  }
  int i;
  for(i = 0; i < argc; i++){
    if((flagptr = strchr(argv[i], '-'))){
      // there is a flag, handle it, and handle the file, I guess
      switch(*(++flagptr)){
        case 'd':
          printf("Debug Mode\n");
          debug_flag = 1;
          if(!(file = fopen(argv[i+1], "r"))){
            printf("No file found follow the flag with the input\n");
            return 0;
          }
          break;
        case 'r':
          printf("RegDump Mode\n");
          reg_dump = 1;
          if(!(file = fopen(argv[i+1], "r"))){
            printf("No file found follow the flag with the input\n");
            return 0;
          }
          break;
        case 'c':
          printf("CacheDump mode\n");
          cache_dump = 1;
          if(!(file = fopen(argv[i+1], "r"))){
            printf("No file found: follow the flag with the input file\n");
            return 0;
          }
      }
      if(*(++flagptr) != '\0'){
        printf("%c\n", *flagptr);
        switch(*flagptr){
          case 'd':
            printf("Debug Mode\n");
            debug_flag = 1;
            break;
          case 'r':
            printf("Regdump Mode\n");
            reg_dump = 1;
            break;
          case 'c':
            printf("Cache dunp Mode\n");
            cache_dump = 1;
            break;
        }
      }
      break;
    }
  }
  if(i == argc){
    // no flags
    if(!(file = fopen(argv[1], "r"))){
      printf("File Not Found. Entered File: %s\n", argv[1]);
      return 0;
    }
  }

  initialSetup(file);

  while(FDEI() != 0){
    if(debug_flag){ printf("\n"); }
  }
  if(reg_dump){
    printf("-----------------\n");
    printf("| Register Dump\t|\n");
    printf("-----------------\n");
    printf("| Reg:\t| Val:\t|\n");
    printf("-----------------\n");

    for(i = 0; i < 16; i++){
      printf("| %d\t| %04x\t|\n", i, reg_file[i]);
    }
    printf("-----------------\n");
  }
  if(cache_dump){
    printf("-------------------------\n");
    printf("| \tCache Dump\t|\n");
    printf("-------------------------\n");
    printf("| Addr:\t| Val:\t| LRU:\t|\n");
    printf("-------------------------\n");
    int j = 0;
    for(i = 0; i < 4; i++){ // per block
      for(j = 0; j < 4; j++){ // per slot
        printf("| %X\t| %04X\t| %01X\t|\n", cache_line[i][j].address, cache_line[i][j].w, cache_line[i][j].lru);
      }
      printf("-------------------------\n");
    }
    printf("-------------------------\n");

  }
  //while(FDEI() != 0xFFFF){ // while PC != RAMEND
  //  if(++i > 3) break; // short stop point
  //}
  return 0;
}

/* Function: initialSetup
 * Description: performs the inital setup for the CPU (setting the SP, PC, Sr, etc)
 * Parameters: the Input s19 file
 * Return: none
 */
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
  // set the SP to RAMEND = 0xFFBF, as default
  MDB_x = 0xFFBF;
  reg(SP, WRITE_rw);

  int i,j;
  // Initialize the cache
  for(i = 0; i < 8; i++){
    for(j = 0; j < 4; j++){
      cache_line[i][j].address = 0;
      cache_line[i][j].lru = 3-j;
    }
  }
}

/* Function: FDEI
 * Description: executes the Fetch, Decode, Execute, Interrupts loop. Checks for stop condition
 * Parameters: none
 * Return: the current Address of the PC, or 0 if an instruction is fetched that = 0
 */
uint16_t FDEI(void)
{
  // Fetch, Decode, Execute, Interrupts
  uint16_t pc;
  fetch();
  pc = MAB_x; // the address fetched
  // the new value is on the MDB, and PC+=2
  // process the new instruction, and determine the operands, etc
  // Returns a fully featured record
  record_t record;
  record = decode();
  if(record.instruction == 0){
    printf("All Done!\n");
    return 0;
  }
  // Now that we have the id of the inst, its operand(s), and its
  // area of execution: can actually execute that code. 
  // Jumps are now just addition, and everything is either handled
  // by the ALU, external ALU hardware, or by the ID itself (SP/ID)
  execute(record);
  return pc;

}
