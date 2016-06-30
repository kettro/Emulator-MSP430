// Author : Ross Kettleson
// File: alu.c
// Purpose: Compute ALU actions: addition, logic, shifts. 
//          Update the SR after an execution
// Include Files
#include <stdint.h>
#include "structures.h"
#include "macros.h"
#include "lookup_tables.h"
// Defines and Macros
// Local Variables
// Local Function Prototypes
int alu(record_t record, status_reg_t sr);
void updateSR(status_reg_t sr, uint32_t alu_f, record_t record);
// External Variables
extern uint16_t A_x;
extern uint16_t B_x;
extern uint16_t MAB_x;
extern uint16_t MDB_x;
// External Function Prototypes

// Definitions

