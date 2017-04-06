Design Statement

The Following documents serves as an introduction to this Emulator for the MSP430.
It shows the method behind much of the madness, and attempts to give a good
description of the layout and logic in the code, as well as in some of the design
decisions. The structure is as follows: a discussion of the Structure of project;
a note of the various data structures, and their uses; a short overview of the 
basis for the algorthms used; a short discussion on the Cache (As relating to
Assignment 3); finally, a short discussion on the implementation of a Pipeline.

*--Structure
The backbone of the design of this emulator was to attempt to not simulate,
but to emulate the CPU. In order to get the closest and most faithful representation
of the CPU, I determined that the best method to emulate its behaviour was to go
through the same steps that the physical system would. I call this approach "emulation".
This approach contrasts with one that seeks to simulate the CPU, that is, to have the
same input and outputs as the CPU, but operate using an entirely divorced method from
the hardware itself, outside concerns such as opcodes, or memory sizes.

The approach to emulate the CPU has several advantages over the approach to simulate,
however also suffers some drawbacks.
Some drawbacks are:
  * Limited in procedure: For certain operations, such as incrementing the PC after
      a fetch, the ALU must be used in order to actually add 2 to the PC. This means
      that in the period between putting the PC on the ALU and then returning that value
      into the register file, nothing can be placed on the MDB, lest they be overwritten
      by the PC being incremented.
  * Limited in resources: As I will discuss below under Pipline Concerns, This approach
      requires that temporary registers must be used by the various steps in order to
      actually hold the data being transferred around. This is implicitly done in the ID
      using the record_t data structure, but explicitly done using, eg the MDB and MAB.

However, Some strengths include:
  * Compact and streamlined algorithms: Because emulation is designed to mirror the
      machine, operations such as the ALU are done in a way to reduce the complexity
      of the arguments. To take the ALU as an example, it sorts the instructions based on
      whether they are math or logic operations. It also shifts based on Carry, etc, in
      an ettempt to reduce all math instructions to simply addition, and all logic functions
      to AND, OR, and XOR, just like a real ALU would. This results in algorithms that do
      not need to have a large number of functions, that each operate each instruction,
      nor a complicated system of parameters and returns.
  * Reduced data complexity: The emulation approach relies on using the MDB and MAB as the
      main temparary storages for data. As such, all functions that would return a value,
      return it to the MDB. This ensures that at each step in any algorithm, only one new
      piece of data is being considered, and only one piece of data is being handled.
  * Restricted Design: a restricted design, such as in painting inside a tradition, can
      sometimes result in solutions that would not have become apparent to the designer
      who had complete and total freedom. The result here would be in tighter handling of
      data, ordering of executions, and more aggressive simplifications of patterns.
  * Constant State: In the CPU, state is maintained only in the Status register. This means
      that a faithful emulation would also be completely independent of the instructions
      that follow or precede it, outside of the Status Register. This means that state
      is not a concern: per module, the only concern is the values on the MDB/MAB/A/B/F,
      and the current record.

For this project, I decided that the strengths by far outweighed the weaknesses. I was also
excited for the challenge in the task, which provided a non-zero influence on my decision.

The code is lain out in such a way as to emulate the different stages and modules of a CPU:
The Memory, the Instruction Decoder, the Busses, and the ALU, all doing Fetch, Decode, Execute.
Each stage may use all of the modules, but it will only use the modules that are relevant to it.
Additionally, each module is independent from the other modules, outside of some access (the ALU
updating the SR, for example, or the ID requesting memory access). Data flows from the MDB, back
into the MDB. Outside of the record, each module only uses a single argument at most (outside the
SR pointers, done due to restrictions outside the design), and returns nothing. This, again, is
done to both simplify the flow of data, but also to emphasize in the code that each module is
independent: it does its thing, and then stops; the next thing then does its thing, then stops, etc.

*--Data Structures
There are a number of data structures used in this project. They fall cleanly into three camps:
Enums, Structs and Unions, and Lookup Tables.
The Enums are simple shorthands for commonly used and compared values that map to integers.
The Structs and Unions are Data of a determined size, with determined members, each of their own
types. As an example, the record_t type is a determination of the various elements of a record:
the addresses, opcodes, and addressing modes, grouped together as a block that can be passed
to different modules. 
The Lookup Tables are similar to statically defined functions. They return a value, based on
a given index. As an example, the table for determining if an instruction uses the carry takes
as an index the instruction's opcode, and returns a boolean.

A more complete handling of the Data Structures is handles after this introduction.

*--Algorithms
The main algorithm of the Emulator is the simple FDE sequence. The emulator must fetch, then
decode, and then execute. This was, over a number of iterations, further and further defined
into the constituent parts, which were defined into more parts, as needed.
In Fetch, the PC must be used to retrieve the next item in program memory. It must also increment
the PC by the word size, 2 bytes. In order to do this, fetch must get the PC, and put it on the 
MAB- the Address bus, where it waits for the memory to be called. However, fetch then puts the PC
on the ALU with 2, and then puts the result from the MDB into the PC register. Only then is the
memory called, and the "old" value of the PC used to retrieve the desired memory.
For Decode, The instructions must be seperated based on their Opcode types (Type 1, 2 or jumps),
and then on their opcodes. These two numbers uniquely identify an instruction (thus, Type 1, opcode
5 is not the same as type 2 opcode 5). If it is a Type 1 or a type 2 instruction, then the
Addressing modes for the instruction must then be determined, in order to know where to look
for the arguments. This may result in further calls to fetch in order to get values from the memory.
For jumps, the offset must be determined by a shift and a sign extension. To keep the state of the
machine, the record is altered to be an addition record, which has an argument the PC and the offset,
if the jump is taken (therefore, as far as the ALU is converned, the instruction is simply addition).
Depending on the instruction, the Area of Execution is also determined. This allows instructions
to be seperated based on "where" in the CPU they are actually executed. AOE is either the ALU, the ID,
or the SP. ID is currently only the MOV instruction. However, as discussed below, it could be expanded.
SP handles the functions that use the Stack Pointer to operate: RETI, CALL, and PUSH. the ALU handles
all others (though only some by the ALU proper). 
Execution is based on the AOE previously determined. in the ALU, type 2 instructions (addition or logic)
are handled by the ALU directly. The other functions (the shifts RRC, SWPB, RRA, and SXT) are handled
by "external circuitry" in the ALU. For the ID, the data is ferried inside the ID. For the SP AOE, the
memory is called to retrive the item in the location given by the SP, or stored there, depeding on the
instruction.

*--Cache
A Modification to the standard memory system of simply looking in the memory file for the desired
location in memory, the cache in real machines allows for much much much more rapid memory access.
Its implementation is the task of Assignment 3. I present it here as a replacement for the standard
memory decoder, as it does not remove functionality when unneeded, and adds it when needed. In the
code, only the mem() function is affected.
In building the design of the cache, the standard design done up to here was partially ignored,
simply due to the limitations that emulation by code results in vs the actual hardware. Operating
in parallel is standard for circuitry, but less so for code. The result is code that is less close
to the actual implementation in the hardware.
The cache was designed to be a 32-byte starage device. However, my implementation is not based on
the byte level, but on the word level. Therefore, it has 16 locations (4x4), but 32 byte locations.
This was done for ease of use on the coder's level, not for fidelity to the CPPU, as a consession.

Caches can be implemented using two expremes of a spectrum, or shades in between. The two strategies
are Direct and Associative mapping. DM uses a hash in order to determine which "slot" a memory 
location should be stored in. When another piece of memory is accessed that hashes to the same slot,
then the old slot is discarded. AM uses a linear search in order to first try to find the location,
and then, if there is no hit, find the eldest- or Least Recently Used- and replace that, while
incrementing the LRU of all slots younger. 
This Cache, in contrast, operates on a mixture. It uses 4 blocks of 4 words. On order to allow
for the blocks to be filled fully, the hash used divides the memory location by 2, then takes the
modulus with the number of blocks, 4. regardless of whether the cache is being read or written to,
a new cache line is created with the correct values. Then, a linear, AM search is used in order to 
find the correct line. If it results in a miss, the another search in the block is done for the LRU.
That item is then replaced with the new cache line. If the action was a read, then the new line is
returned onto the MDB. If it was a write, then the Dirty Bit is checked, to determine if something
has been written to. If it is set, then the old cache entry needs to be written through to memory.

*--Pipeline Concerns
The Pipeline in the 430 is relatively infeasible, due primarily to the uneven clock counts of the
code. In order to implement a Pipeline effectively, a new instruction must be introduced, a
conditional, IFcc, where cc is the condition. This allows for certain things to occur:
  * allows to avoid bubbling, by saying that the code should only execute instructions if they are
      desired.
  * Removes complexity from the ISA: It allows the removal of jumps from the ISA, resulting in 
      only 21 instructions. JMP would be maintained, but the others could be removed adn replaced by a
      IFcc instruction, and a JMP. 
  * Requires state to be preserved: In order for the IFcc to track however many instructions to
      execute if true/false. This requres some state to be saved. This is a problem, as my system is
      almost entirely memoryless. 

To solve the problem of state, I devised a plan to drop away all of the jump instructions, save JMP.
In the place, an IFcc instruction of the format:
    |___|__|__|_________|
      ^    ^       ^
  opcode  T/F  condition
         count 

This is paired up the a new Status register, that has the form:
    |__|_|__|__|_________|
        ^   ^
     c_act TC/FC

Where c_act is a flag of whether a condition is active or not, and the TC/FC are counts of however
many instructions are left to execute per side. The operation of these two structures is that the
conditions are lined up with the status bits in the current SR. The table below of conditions gives
the mappings:

-----------------------
| Cond | JmpOp | Code |
-----------------------
| NZ   | 000   | 002  |
| Z    | 001   | 012  |
| NC   | 010   | 001  |
| C    | 011   | 011  |
| N    | 100   | 014  |
| GE   | 101   | 104  |
| L    | 110   | 114  |
-----------------------

Where in the codes, The middle nibble is the expected value of the comparison. The bit is in the 
location of the C_off SR bit, which controls if the CPU is active or not. The desired result can
be determined by the AND of C_off and this new middle bit. The high and low bytes can AND with the
SR, with the result being compared to the desired outcome, which is fed back into c_active.
