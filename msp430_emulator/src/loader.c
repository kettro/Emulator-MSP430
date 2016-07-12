// Author : Ross Kettleson
// File: loader.c
// Purpose: to load S19 files into memory

// Include Files
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Defines and Macros
// Local Variables
// Local Function Prototypes
uint16_t loader(FILE* input_file);
// External Variables
extern uint16_t memory[0x10000]; // doesn't use the mem fns
// External Function Prototypes

// Definitions

// returns: the start location, aka, first location in the program
uint16_t loader(FILE* input_file)
{
  // input file is the .s19 record
  //
  uint16_t checksum = 0;
  uint16_t start_address;
  uint16_t address;
  int first_line = 1;
  int stated_len;
  int str_len;
  char line[80]; // a few extra spots- max of 72 per line
  char* ptr;
  char* saved_ptr;
  char b[2];
  char w[4];
  uint8_t byte;
  while(fgets(line, sizeof(line), input_file)){
    ptr = line;
    checksum = 0;
    if(!(*ptr == 'S' && *(++ptr) == '1')){ // verify the leader
      // error
    }
    // get the length
    b[0] = *(++ptr);
    b[1] = *(++ptr);
    str_len = (strlen(line) - 4)/2;
    stated_len = strtol(b, NULL, 16);
    if(!(str_len == stated_len)){
      // error
    }
    checksum += stated_len;

    // get the address
    w[0] = *(++ptr);
    w[1] = *(++ptr);
    w[2] = *(++ptr);
    w[3] = *(++ptr);
    w[4] = '\0';
    address = strtol(w, NULL, 16);
    checksum += address & 0xFF;
    checksum += (address >> 8) & 0xFF;
    if(first_line){ // asssign the first address
      first_line = 0;
      start_address = address;
    }
    // calculate the checksum
    saved_ptr = ptr;
    int i;
    b[2] = '\0';
    for(i = 0; i < str_len - 2; i++){ // -8 to skip addr, len, S1, not chksum
      b[0] = *(++ptr);
      b[1] = *(++ptr);
      byte = strtol(b, NULL, 16);
      checksum += byte;
    }
    checksum = ~(checksum & 0xFF);
    if(checksum != byte){
      // error - checksums must match
    }
    // actually get the values now
    ptr = saved_ptr; // recall the ptr
    for(i = 0; i < str_len - 3; i++){ // -10 to skip addr, len, S1, and checksum
      b[0] = *(++ptr);
      b[1] = *(++ptr);
      byte = strtol(b, NULL, 16);
      memory[address] = byte;
      address++;
    //  printf("in address %x, entering value %02x\n", address - 1, byte);
    }
  }
  return start_address;
}
