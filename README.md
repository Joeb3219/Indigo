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
 * 0x01 - ADD: a b ADD: Pushes a + b to the stack.
 * 0x02 - SUBT: a b SUBT: Pushes a - b to the stack.
 * 0x03 - MULT: a b MULT: Pushes a * b to the stack.
 * 0x04 - DIV:  a b DIV : pushes a / b to the stack.
 * 0x05 - MOD: a b MOD: Pushes a % b to the stack.
 * 0x06 - RAND: RAND: Pushes a random number to the stack
 * 0x07 - PRINT: a PRINT: Prints a to console.
 * 0x08 - END: END: Ends the current block.
 * 0x09 - GET_REGISTER: a GET_REGISTER: Pushes the value of register a to the stack.
 * 0x0a - SET_REGISTER: a b SET_REGISTER: Sets the value of register a to b.
 * 0x0b - GET_REGISTER_COUNT: GET_REGISTER_COUNT: Pushes the number of Registers to the stack.
 * 0x0c - OR OPERATOR: a b OR: Pushes a || b to the stack (eg: T || F => T, F || F => F).
 * 0x0d - XOR OPERATOR: a b XOR: Pushes a X|| b to the stack (eg: T || F => T, F || F => F, T || T => F)
 * 0x0e - AND OPERATOR: a b AND: pushes a && b to the stack (eg: T && T => T, T && F => F, F && F => F).
 * 0x0f - NOT OPERATOR: a NOT: Pushes the inverse of a (eg: !T => F, !F => T).
 * 0x10 - EQUALS OPERATOR: a b EQUALS: Pushes a == b (eg: if a == b, pushes 1, otherwise pushes 0).
 * 0x11 - GREATER THAN: a b GREATER THAN: Pushes a > b
 * 0x12 - LESS THAN: a b LESS_THAN: pushes a < b
 * 0x13 - IF: a IF: continues in block IFF a is equal to 1.
 * 0x14 - SET_MARKET: a SET_MARKET: Sets a marker at position a to the current instruction.
 * 0x15 - JUMP_TO: a JUMP_TO: Jump to the instruction specified by marker[a].
 * 0x1a - CHAR: a CHAR : specifies that the previously pushed byte is a character and should be converted to a char when printed.
 * 0x1b - PRINT_LONG: a PRINT_LONG: prints the last a stack objects.
 * 0x1c - LETTER: a b LETTER: Pushes a letter to the stack that is uppercase if a == 1, beginning at b (eg: b == 0 => a, b == 1 => b).
 * 0x1d - READ_FILE: a b READ_FILE: reads in instructions from file named a.txt starting at register b.
 * 0x1e - FUNCTION_DECLARE: a b FUNCTION_DECLARE: Creates a function with ID a, which takes the last b parameters pushed to the stack.
 * 0x1f - FUNCTION_JUMP: a FUNCTION_JUMP: Jumps to a function with the id a
 * 0x20 - RETURN: a RETURN: Pushes a to the stack after function execution ends
