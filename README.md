# Indigo
A "programming language" with a compiler written in C

A "language" written to observe program execution in multiple langauges. The initial compiler is written in C, and future compiler version will be written in Indigo. 

Indigo currently uses 4-digit hex values to send instructions to the compiler. The end goal is to complete a language (Indigo Prime), written in Indigo, which parses text documents written in English and is able to execute OOP principles. 

Langauge specifics:

 * Indigo uses a stack to process all instructions, which can be read from a text file.
 * The Indigo Compiler uses 8192 memory registers to store various data.
 * Each register has a marker which indicates whether or not it is a "special" value - some values stored in registers are characters, IF statements, etc.
 * By marking a register as "special", we are able to differentiate different types of data in the system. For example, when PRINT is called, the compiler checks if the identifier "1" is used, in which case it uses the character represented by the value of the register.
 
Current instructions accepted:

 * 0x00 - LITERAL: literal a: Pushes a to the stack
 * 0x01 - Add: a b Add
 * 0x02 - Subtract: 
 * 0x03 -
 * 0x04 -
 * 0x05 -
 * 0x06 -
 * 0x07 -
 * 0x08 -
 * 0x09 -
 * 0x0a -
 * 0x0b -
 * 0x0c -
 * 0x0d -
 * 0x0e -
 * 0x0f -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 * 0x10 -
 
