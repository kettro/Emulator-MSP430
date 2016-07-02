// Author : Ross Kettleson
// File: decode.c
// Purpose: Contains functions to interpret the incoming instructions

// Include Files
#include <stdint.h>
#include "structures.h"

// Defines and Macros
// Local Variables
// Local Function Prototypes
void parseOpcode(record_t record);
void parseOperand(record_t record);
void calculateOffset(record_t record);
void calculateOperandAddress(record_t record);
// External Variables
extern uint16_t A_x;
extern uint16_t B_x;
extern uint16_t MAB_x;
extern uint16_t MDB_x;
// External Function Prototypes
extern int alu(record_t record, status_reg_t* sr);

// Definitions

