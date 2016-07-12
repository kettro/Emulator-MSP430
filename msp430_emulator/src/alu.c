// Author : Ross Kettleson
// File: alu.c
// Purpose: Compute ALU actions: addition, logic, shifts.
//          Update the SR after an execution
// Include Files
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "structures.h"
#include "macros.h"
#include "lookup_tables.h"
// Defines and Macros
// Local Variables
uint32_t F_l;
// Local Function Prototypes
int alu(record_t record, status_reg_t* sr);
void updateSR(status_reg_t* sr, record_t record);
uint32_t decimalAdd(uint16_t src, uint16_t dst);
// External Variables
extern uint16_t A_x;
extern uint16_t B_x;
extern uint16_t MAB_x;
extern uint16_t MDB_x;
// External Function Prototypes
extern void reg(Register_e reg_index, ReadWrite_e rw);

// Definitions

int alu(record_t record, status_reg_t* sr)
{
  int carry_val;
  uint16_t temp = 0x00;
  int counts;
  Data_t src;
  Data_t dst;
  src.w = A_x;
  dst.w = B_x;
  uint16_t bitmask = 0x0001;
  uint16_t f_mask = 0xFFFF; // for masking F_l at the end
  int i;

  if(sr != NULL){
    carry_val = sr->c;
  }else{
    carry_val = 0;
  }

  if(record.bw == BYTE_bw){
    f_mask = 0x00FF;
    src.w &= f_mask;
    dst.w &= f_mask;
  }

  if(record.op_type == ONE_opt){ // is type 1: shift!
    // handle swpb, rra, rrc
    switch(record.opcode){
      case SWPB_op:
        F_l = (src.bh | src.bl<<8);
        break;
      case RRC_op:
        temp = carry_val; // save the carry
      case RRA_op: // fall through: handled similarly
        if(record.bw == BYTE_bw){ counts = 8; }
        else{ counts = 16; }
        carry_val = src.b0;
        if(sr != NULL){ sr->c = carry_val; }
        for(i = 1; i < counts; i++){
          bitmask = bitmask << 1; // mask for the next left bit
          F_l |= ((src.w & bitmask) >> 1); // right shift the specified bit
        }
        if(record.opcode == RRC_op){
          F_l |= temp << counts; // put carry into the MSB
        }
        break; // on to updateSR
      case SXT_op:
        // check the sign bit of the src, and
        if(BIT7(src.w)){ // is neg
          F_l = (0xFF00 | src.w); // make high byte all 1's
        }else{ // is pos
          F_l = (0x00FF & src.w); // make high byte all 0's
        }
        break;
      default:
        //error: shouldn't be other insts here
        break;
    }
  }else if(MathShiftLogic_lt[record.opcode] == MATH_msl){ // a math op
    // handle MATH
    if(Carry_lt[record.opcode] == 1){ // this opcode useses a carry
      if(NegSub_lt[record.opcode]){ src.w = ~(src.w) + carry_val; } // subtraction
      else{ src.w += carry_val; } // addition
    }else { // no carry
      if(NegSub_lt[record.opcode]){ src.w = ~(src.w); } // subtraction
    }

    if(record.opcode == DADD_op){ F_l = decimalAdd(src.w, dst.w); }
    else{ F_l = (uint32_t)src.w + (uint32_t)dst.w; } // cast as 32b to allow for carry
    // on to updateSR
  }else if(MathShiftLogic_lt[record.opcode] == LOGIC_msl){ // a logic op
    // handle LOGIC
    if(record.opcode == BIC_op){ src.w = ~(src.w); }
    switch(LogicOperation_lt[record.opcode]){
      case AND_lo: // BIT,BIC,AND
        F_l = src.w & dst.w;
        break;
      case OR_lo: // BIS
        F_l = src.w | dst.w;
        break;
      case XOR_lo: // XOR
        F_l = (src.w | dst.w) & ~(src.w & dst.w); // XOR = (A|B) & ~(A&B)
        break;
      default:
        //error: all viable cases handled above;
        break;
    }
    // on to updateSR
  }else{
    printf("Error in ALU:\n");
    printf("Opcode %04x, Type %d\n", record.opcode, record.op_type);
    printf("SRC = %04x, DST = %04x\n", src.w, dst.w);
    printf("Undefined Behavior\n");
    // error: undefined behavior
  }
  updateSR(sr, record);
  if(record.op_type == TWO_opt && (record.opcode == CMP_op || record.opcode == BIT_op)){
    return 0;
  }else{
    MDB_x = F_l & f_mask; // put F on the MDB as a byte or a word
    return 1;
  }

}

void updateSR(status_reg_t* sr, record_t record)
{
  if(sr == NULL){ return; } // used to do no-time increments (PC+2, etc)
  int sign_f;
  int sign_a;
  int sign_b;
  int Carry = 0; // assume a default of 0 for all statuses
  int Negative = 0;
  int Zero = 0;
  int Overflow = 0;
  int GIE = 0;
  // Carry?
  if(record.bw == BYTE_bw){ // is a byte
    if(F_l == (F_l & 0xFF)){ Carry = 1; } 
    sign_f = BIT7(F_l);
    sign_a = BIT7(A_x);
    sign_b = BIT7(B_x);
  }else{ // Word
    if(F_l == (F_l & 0xFFFF)){ Carry = 1; }
    sign_f = BIT15(F_l);
    sign_a = BIT15(A_x);
    sign_b = BIT15(B_x);
  }
  // Negative
  if(sign_f == 1) { Negative = 1; }
  // Zero
  if(F_l == 0){ Zero = 1; }
  // Overflow? // this is the easiest way to calculate: -- => + and ++ => -
  // V = (!F & A AND B) | (F & !(A OR B));
  // using !x in order to make a boolean of the value
  Overflow = (((!sign_f) & sign_a & sign_b) | (sign_f & !(sign_a | sign_b)));
  // GIE?
  // Only Reti changes the GIE, outside interrupts
  GIE = ((record.opcode == RETI_op) && (record.op_type == ONE_opt));

  sr->c = Carry;
  sr->z = Zero;
  sr->n = Negative;
  sr->v = Overflow;
  sr->gie = GIE;
  sr->scg0 = sr->scg0;
  sr->scg1 = sr->scg1;
  sr->coff = sr->coff;
  sr->ooff = sr->ooff;
  // put sr on the MDB
  MDB_x = sr->w;
  reg(SR, WRITE_rw);
  // TODO: increment the SYSclock
  return;
}

uint32_t decimalAdd(uint16_t src, uint16_t dst)
{
  // only unsigned addition
  // add decimally - External hardware
  // if there is a carry, it has been added above: 0x9 + 0x9 + 1 == 0x9 + 0xA
  int i;
  int carry = 0;
  uint32_t result = 0;
  uint16_t bitmask = 0x000F;
  int s_nibble;
  int d_nibble;
  int result_nibble;
  for(i = 0; i < 4; i++){
    s_nibble = (src >> (i * 4)) & bitmask; // left shift 0, 4, 8, 12 spots
    d_nibble = (dst >> (i * 4)) & bitmask; // left shift 0, 4, 8, 12 spots
    result_nibble = s_nibble + d_nibble + carry;
    if(result_nibble > 9){
      carry = (int)(result_nibble / 10); // get 10's parts
      result_nibble = result_nibble % 10; // get 1's part
    }
    else{ carry = 0; } 
    result |= (result_nibble << (i * 4)); // put the nibble in the right place
  }
  result += carry << 16; // add in last carry to the result
  return result;
}
