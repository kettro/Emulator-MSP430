# Emulator-MSP430

An emulator for TI's classic 16-bit microcontroller, the MSP-430.

## Structure

The emulator is built in a similar manner as microcode. Individual components
of the CPU are emulated, and given their own distinct module. These components
are as follows:

* Instruction Decoder
* Arithmetic Logic Unit
* Main Data Bus, Main Memory Bus
* Register File
* Memory
* Cache

## Instructions and Code

Instructions can be generated using the Assembler-MSP430, or can be
hand-written. Code is in the format of an .s19 file (examples can be seen in
the test folders). Files are accepted by the loader, and executed.
The format of instructions are the same as that of the standard MSP-430
instruction set.

## Omitted Features
Currently, all instructions are implemented,and are executed in a cycle
correct manner. However, Interrupts are not fully implemented, due to
the conceptual difficulty in implementing external devices. Additionally,
pipelining is yet to be implemented, and as such, the system has no method
for branch prediction, or any similar optimizations.
