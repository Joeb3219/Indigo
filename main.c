#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

/**
 * INDIGO COMPILER V1.0.6
 * INDIGO IS A LANGUAGE WHICH UTILIZES HEX VALUES FOR ALL INSTUCTIONS.
 * WRITTEN BY JOSEPH BOYLE (joeb3219).
 * Indigo uses a stack to process all instructions, which can be read from a text file.
 * The Indigo Compiler uses 8192 memory registers to store various data.
 * Each register has a marker which indicates whether or not it is a "special" value - some values stored in registers are characters, IF statements, etc.
 * By marking a register as "special", we are able to differentiate different types of data in the system. For example, when PRINT is called, the compiler checks if the identifier "1" is used,
 * in which case it uses the character represented by the value of the register.
 *
**/

static int instructions[1024 * 8] = {-1}; //A list of instructions to process in the application.
static int stack[1024 * 2] = {-1}; //The stack which is used to push values to other segments of code.
static int specialIdentifierAtStackPosition[1024 * 2] = {};
static int tail = -1; //This is the last used position on the stack (eg, after one insert, tail will be set to 0).
static int registers[1024 * 8] = {}; //The registers which data may be set/retrieved to/from during program execution. IE: Memory.
static int markers[1024 * 8] = {}; //Used to tell us what kind of data a value in a register actually represents (eg: setting
static int functions[128][2] = {}; //A list of functions stored by the program. functions[][0] contains the instruction number, and [][1] contains the number of args.
static int lineNumbers[sizeof(instructions) / sizeof(instructions[0])] = {};
static int currentInstruction = 0;

const int _IDENTIFIER_FUNCTION_DECLARE = -132; //A flag used to identify a function declaration, which we ignore for now.
const int _IDENTIFIER_FUNCTION = -131; //A flag that to identify when we were in a function call.
const int _IDENTIFIER_FAILED_IF = -130; //A flag that, if pushed the the stack, will cause the program to cease execution of instructions until an END statement is met.
const int _IDENTIFIER_GOOD_IF = -129; //A flag used to identify IF conditions which the program is to go inside of.
const int _IDENTIFIER_CHAR = -128; //A flag used to identify that the register represents a character.

int main(int argc, char *argv[]){
    FILE *file;
    int k = 0;

    for(k = 0; k < sizeof(registers) / sizeof(registers[0]); k ++) registers[k] = INT_MIN + 1;

    printf("Hello world! I am a Virtual Machine!\n");

    file = fopen("compiler.txt", "r");

    k = processFile(file, instructions, 0);

    fclose(file);

    interpret(k);

    return 0;
}

//Processes a file, returning the total number of instructions extracted from the file.
int processFile(FILE* file, int* theseInstructions, int startRegister){
    char line[4096] = {0};
    char processedLine[4096] = {0};
    char substring[11]; //Allows for an instruction with a maximum value of 0xffffffff plus an \0 character.
    int val = 0, i = 0, j = 0, start = 0, k = startRegister, lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum ++;
        j = 0;
        for(i = 0; i < sizeof(line); i ++){ //Copy over non-space, non EOL characters to be processed.
            if(line[i] == '/' || line[i] == '\n'){
                i = sizeof(line);
                continue;
            } else if(line[i] != ' '){
                processedLine[j++] = line[i];
            }
        }

        start = 0;
        //To allow for instructions of up to 0xffffffff, we scan for occurrences of 0x and slice the string accordingly.
        //We begin at i=2 so as to skip the first 0x occurrence, and continue until one past the end of the string so we can calculate the ending bounds correctly.
        for(i = 2; i <= strlen(processedLine) + 1; i ++){
            if(i != (strlen(processedLine) + 1) && processedLine[i] != 'x') continue;
            memcpy(substring, &processedLine[start], (i - 1) - start); //Copy over single instruction to a substring
            substring[(i - 1) - start] = '\0';
            val = (int) strtol(substring, NULL, 0); //Convert substring to an decimal integer value
            lineNumbers[k] = lineNum; //Add line numbers
            theseInstructions[k++] = val; //Add instruction to the list
            start = i - 1;
        }
        memset(processedLine, 0, sizeof(processedLine)); //Clear out the line so we don't reread larger instructions in the next cycle.
    }
    return k;
}

int readFileIntoMemory(char* fileName){
    FILE *file;
    char line[2048] = {0};
    int i = 0, j = 0, lengthOfFile = 0, actualRegistersUsed = 0;

    file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
    lengthOfFile = ftell(file);
    rewind(file);
    lengthOfFile += 3; // Request 3 more than length of file for the array descriptors.

    // Get the correct number of registers
    for(j = readFromRegister(1); j < sizeof(registers) / sizeof(registers[0]); j ++){
        if(i == lengthOfFile) break;
        if(registers[j] == readFromRegister(7)) i ++;
        else i = 0;
    }

    j -= lengthOfFile;

    storeInRegister(j, readFromRegister(16)); //Push the ARRAY_SYMBOL to memory.
    storeInRegister(j + 1, readFromRegister(18)); //Indicate that this is a character array via CHAR_SYMBOL.
    j += 2;
    actualRegistersUsed += 2;

    while (fgets(line, sizeof(line), file)) {
        for(i = 0; i < sizeof(line); i ++){ //Copy over non-space, non EOL characters to be processed.
            storeInRegister(j++, line[i]);
            actualRegistersUsed ++;
            if(line[i] == '\n'){
                i = sizeof(line);
                continue;
            }
        }
    }

    storeInRegister(j++, readFromRegister(19)); //Indicate that the array is finished
    actualRegistersUsed ++;

    fclose(file);

    return j - actualRegistersUsed;
}

void interpret(int numInstructions){
    int a, b = 0;
    for(currentInstruction = 0; currentInstruction < numInstructions; currentInstruction ++){
        if(readFromRegister(3) == -129) return;
        //If the current item on the stack is in fact an if statement which we aren't entering, only consider other IF/END statements
        if(peek() == _IDENTIFIER_FAILED_IF && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FAILED_IF){
            if(instructions[currentInstruction] == 0x00){ // Setting up for a literal, disregard it.
                currentInstruction ++;
                continue;
            }
            if(instructions[currentInstruction] == 0x13){
                push(_IDENTIFIER_FAILED_IF);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FAILED_IF;
            }else if(instructions[currentInstruction] == 0x08) pop(); //Pops off the IF call.
            continue;
        }else if(peek() == _IDENTIFIER_FUNCTION_DECLARE && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION_DECLARE){
            if(instructions[currentInstruction] == 0x00){ // Setting up for a literal, disregard it.
                currentInstruction ++;
                continue;
            }
            if(instructions[currentInstruction] == 0x13){
                push(_IDENTIFIER_FAILED_IF);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FAILED_IF;
            }
            if(instructions[currentInstruction] == 0x08) pop(); //Pops off the FUNCTION_DECLARE call.
            continue;
        }
        switch(instructions[currentInstruction]){
            case 0x00: //LITERAL: literal a: Pushes a to the stack
                currentInstruction ++;
                push(instructions[currentInstruction]);
                break;
            case 0x01: //ADD: a b ADD: Pushes a + b to the stack.
                b = pop();
                a = pop();
                push(a + b);
                break;
            case 0x02: //SUBT: a b SUBT: Pushes a - b to the stack.
                b = pop();
                a = pop();
                push(a - b);
                break;
            case 0x03: //MULT: a b MULT: Pushes a * b to the stack.
                b = pop();
                a = pop();
                push(a * b);
                break;
            case 0x04: //DIV:  a b DIV : pushes a / b to the stack.
                b = pop();
                a = pop();
                push(a / b);
                break;
            case 0x05: //MOD: a b MOD: Pushes a % b to the stack.
                b = pop();
                a = pop();
                push(a % b);
                break;
            case 0x06: //RAND: RAND: Pushes a random number to the stack
                push(4);
                break;
            case 0x07: //PRINT: a PRINT: Prints a to console.
                printf("[VM]: ");
                if(specialIdentifierAtStackPosition[tail] == _IDENTIFIER_CHAR) printf("%c\n", (char) pop());
                else printf("%d\n", pop());
                break;
            case 0x08: //END: END: Ends the current block.
                b = getPosOfRecentStateChange(); // Find the closest conditional/state changing item in the stack.
                if(b == -1) error("End cannot be matched to an opening call.\n");
                if(stack[b] == _IDENTIFIER_FUNCTION){
                    popTarget(b, 1);
                    storeInRegister(4, readFromRegister(4) - functions[pop()][1] - 1); // Decrement the function argument offset register by the number of arguments this function took.
                    currentInstruction = pop(); //Return to calling instruction.
                }else popTarget(b, 0);
                break;
            case 0x09: //GET_REGISTER: a GET_REGISTER: Pushes the value of register a to the stack.
                a = pop();
                push(readFromRegister(a));
                break;
            case 0x0a: //SET_REGISTER: a b SET_REGISTER: Sets the value of register a to b.
                b = pop();
                a = pop();
                storeInRegister(a, b);
                break;
            case 0x0b: //GET_REGISTER_COUNT: GET_REGISTER_COUNT: Pushes the number of Registers to the stack.
                push(sizeof(registers));
                break;
            case 0x0c: //OR OPERATOR: a b OR: Pushes a || b to the stack (eg: T || F => T, F || F => F).
                b = pop();
                a = pop();
                if(a == 1 || b == 1) push(1);
                else push(0);
                break;
            case 0x0d: //XOR OPERATOR: a b XOR: Pushes a X|| b to the stack (eg: T || F => T, F || F => F, T || T => F)
                b = pop();
                a = pop();
                if(a != b) push(1);
                else push(0);
                break;
            case 0x0e: //AND OPERATOR: a b AND: pushes a && b to the stack (eg: T && T => T, T && F => F, F && F => F).
                b = pop();
                a = pop();
                if(a == b) push (1);
                else push(0);
                break;
            case 0x0f: //NOT OPERATOR: a NOT: Pushes the inverse of a (eg: !T => F, !F => T).
                a = pop();
                if(a == 1) push(0);
                else push(1);
                break;
            case 0x10: //EQUALS OPERATOR: a b EQUALS: Pushes a == b (eg: if a == b, pushes 1, otherwise pushes 0).
                b = pop();
                a = pop();
                if(a == b) push(1);
                else push(0);
                break;
            case 0x11: //GREATER THAN: a b GREATER THAN: Pushes a > b
                b = pop();
                a = pop();
                if(a > b) push(1);
                else push(0);
                break;
            case 0x12: //LESS THAN: a b LESS_THAN: pushes a < b
                b = pop();
                a = pop();
                if(a < b) push(1);
                else push(0);
                break;
            case 0x13: //IF: a IF: continues in block IFF a is equal to 1.
                a = pop(); // Value to test
                // If we are pushing a failed if to the stack, it is okay to push it to the end since no instructions are evaluated.
                // Therefore, we elect to save some computations by always leaving failed calls at the end of the stack for quicker evaluations.
                if(a == 0){
                    push(_IDENTIFIER_FAILED_IF);
                    specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FAILED_IF;
                }else{
                    //Successful IF statements, however, are pushed as close to a function call as possible.
                    //If there are no function calls on the stack, we push as far to the left as possible.
                    //By doing so, IF statements get access to all of the state (global or local, depending on if there is a function call already in the stack).
                    b = getPosOfRecentStateChange(); // Pull the most recent state change.
                    b ++;
                    pushInPlace(b, _IDENTIFIER_GOOD_IF);
                    specialIdentifierAtStackPosition[b] = _IDENTIFIER_GOOD_IF;
                }
                break;
            case 0x14: //SET_MARKER: a SET_MARKER: Sets a marker at position a to the current instruction.
                a = pop();
                markers[a] = currentInstruction;
                break;
            case 0x15: //JUMP_TO: a JUMP_TO: Jump to the instruction specified by marker[a].
                a = pop();
                currentInstruction = markers[a];
                break;
            case 0x1a: //CHAR: a CHAR : specifies that the previously pushed byte is a character and should be converted to a char when printed.
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_CHAR;
                break;
            case 0x1b: //PRINT_LONG: a PRINT_LONG: prints the last a stack objects.
                printf("[VM]: ");
                a = pop();

                for(a; a > 0; a --){
                    if(specialIdentifierAtStackPosition[tail] == _IDENTIFIER_CHAR) printf("%c", (char) pop());
                    else printf("%d", pop());
                }
                printf("\n");
                break;
            case 0x1c: //LETTER: a b LETTER: Pushes a letter to the stack that is uppercase if a == 1, beginning at b (eg: b == 0 => a, b == 1 => b).
                b = pop();
                a = pop();
                if(a == 1) push(b + 'A');
                else push (b + 'a');
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_CHAR;
                break;
            case 0x1d: //READ_FILE: a READ_FILE: reads in instructions from file named a.txt, pushing the memory address stored in back to the stack.
                a = pop();

                char fileName[12];
                sprintf(fileName, "%d.txt", a);
                b = readFileIntoMemory(fileName);
                push(b); // Push the starting register to the stack
                break;
            case 0x1e: //FUNCTION_DECLARE: a b FUNCTION_DECLARE: Creates a function with ID a, which takes the last b parameters pushed to the stack.
                b = pop();
                a = pop();
                functions[a][0] = currentInstruction;
                functions[a][1] = b;
                push(_IDENTIFIER_FUNCTION_DECLARE);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FUNCTION_DECLARE;
                break;
            case 0x1f: //FUNCTION_JUMP: a FUNCTION_JUMP: Jumps to a function with the id a.
                a = pop(); // Pop the ID of the function we're jumping to.
                // Push arguments onto the registers
                if(a > sizeof(functions) || functions[a][0] == 0) error("Undefined function %d.\n", a);
                for(b = 1; b <= functions[a][1]; b ++){
                    storeInRegister(b + readFromRegister(4) + readFromRegister(3), pop()); //Pops off the last b arguments for use by the function, storing them in appropriate registers.
                }
                storeInRegister(b + readFromRegister(4) + readFromRegister(3), 12345678);
                push(currentInstruction); //Push the current instruction number for use later.
                push(a); //Push the function's id for reference when popping.
                push(_IDENTIFIER_FUNCTION);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FUNCTION;
                currentInstruction = functions[a][0];
                storeInRegister(4, readFromRegister(4) + functions[a][1] + 1); //Increase the offset for functions, stored in register 4.
                break;
            case 0x20: //RETURN: a RETURN: Pushes a to the stack after function execution ends.
                b = specialIdentifierAtStackPosition[tail];
                a = pop(); //Save the value which we wish to remain on the stack
                while(!( (peek() == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION) ) ){
                    if(tail == -1) break;
                    pop(); //Until we get to a condition we want, let's keep popping values.
                }
                pop(); // Remove the function call from the stack
                storeInRegister(4, readFromRegister(4) - functions[pop()][1] - 1); // Decrement the function argument offset register by the number of arguments this function took.
                currentInstruction = pop(); //Return to calling instruction.
                push(a); // Push a back onto the stack
                specialIdentifierAtStackPosition[tail] = b;
                break;
            case 0x21: //DUPLICATE_ON_STACK: a DUPLICATE_ON_STACK: Adds a to the stack again (so there will be two a instead of one a).
                a = pop();
                push(a);
                push(a);
                break;
            case 0x22: //VOID_RETURN: VOID_RETURN: FUNCTIONS AS RETURN (0x20), BUT PUSHES NOTHING TO THE STACK.
                while(!( (peek() == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION) ) ){

                    if(tail == -1) break;
                    pop(); //Until we get to a condition we want, let's keep popping values.
                }
                pop(); // Remove the function call from the stack
                storeInRegister(4, readFromRegister(4) - functions[pop()][1] - 1); // Decrement the function argument offset register by the number of arguments this function took.
                currentInstruction = pop(); //Return to calling instruction.
                break;
            case 0x23: //PRINT_LONG_CHARS: a PRINT_LONG_CHARS: Prints the last a stack objects as characters
                printf("[VM]: ");
                a = pop();

                for(a; a > 0; a --){
                    printf("%c", (char) pop());
                }
                printf("\n");
                break;
            case 0x24: //PRINT_STACK: PRINT_STACK: Prints the stack
                printStack();
                break;
            case 0x25: //PRINT_MEMORY: PRINT_MEMORY: Prints the memory
                printMemory();
                break;
        }
    }
}

int readFromRegister(int reg){
    if(reg >= (sizeof(registers) / sizeof(registers[0])) || reg < 0) error("Attempted to read an invalid register: %d.\n", reg);
    return registers[reg];
}

void storeInRegister(int reg, int val){
    if(reg >= (sizeof(registers) / sizeof(registers[0])) || reg < 0) error("Attempted to write to an invalid register: %d.\n", reg);
    registers[reg] = val;
}

int getPosOfRecentStateChange(){
    int i = 0;
    for(i = tail; i > -1; i --){
        if( (stack[i] == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[i] == _IDENTIFIER_FUNCTION)
            || (stack[i] == _IDENTIFIER_GOOD_IF && specialIdentifierAtStackPosition[i] == _IDENTIFIER_GOOD_IF) ){
                return i;
            }
    }
    return -1;
}

//Returns the object that would next be popped from the stack without actually popping it.
int peek(){
    if(tail < 0) return -1;
    return stack[tail];
}

//Pops the indicated target off of the stack.
//If deleteRemainder is set to 1, it will then delete everything to the right of target.
//If deleteRemainder is set to 0, it will then shift everything to the right of target left one place.
void popTarget(int target, int deleteRemainder){
    int b = 0;
    stack[target] = -1;
    specialIdentifierAtStackPosition[target] = 0;
    if(deleteRemainder == 1){
        for(b = tail; b >= target; b --){
            pop();
        }
    }else{
        for(b = target; b < tail; b ++){
            stack[b] = stack[b + 1];
            specialIdentifierAtStackPosition[b] = specialIdentifierAtStackPosition[b + 1];
        }
        tail --;
    }
}

//Pops an object from the stack.
//Also resets the special registers back to zero.
int pop(){
    if(tail < 0){
        error("Attempting to pop an empty stack.\n");
        return -1;
    }
    specialIdentifierAtStackPosition[tail] = 0; //Reset back to being not special
    int val = stack[tail];
    stack[tail] = -1;
    tail --;
    return val;
}

void pushInPlace(int pos, int val){
    int b = tail;
    for(b; b >= pos; b -- ){
        stack[b + 1] = stack[b];
        specialIdentifierAtStackPosition[b + 1] = specialIdentifierAtStackPosition[b];
    }
    stack[pos] = val;
    tail ++;
}

//Pushes an element onto the top of the stack.
void push(int val){
    tail ++;
    stack[tail] = val;
}

//If an error if ever thrown, stop execution of the program.
void error(char* message, ...){
    fprintf(stderr, "Error on line %d.\n", lineNumbers[currentInstruction]);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    printCallTrace();
    printStack();
    //printMemory();
    exit(1);
}

void printMemory(){
    int i = 0;
    printf("MEMORY DUMP");
    for(i; i < (sizeof(registers) / sizeof(registers[0])); i ++){
        if(i % 8 == 0) printf("\n%5d: ", i);
        printf("[%08x]", registers[i]);
    }
    printf("\nEND MEMORY DUMP\n");
}

void printStack(){
    int i = 0;
    printf("STACK: [");
    for(i; i <= tail; i ++){
        if(specialIdentifierAtStackPosition[i] >= 0) printf("%d, ", stack[i]);
        else printf("(S) %d, ", stack[i]);
    }
    printf("]\n");
}

void printCallTrace(){
    int i = tail;
    printf("===== CALL TRACE =====\n");
    printf("On line %d, instruction called: %d (L: %d, R: %d).\n", lineNumbers[currentInstruction], instructions[currentInstruction], instructions[currentInstruction - 1], instructions[currentInstruction + 1]);
    for(i; i > -1; i --){
        if(specialIdentifierAtStackPosition[i] == _IDENTIFIER_FUNCTION && stack[i] == _IDENTIFIER_FUNCTION){
            printf("Function %d, called from line %d.\n", stack[i - 1], lineNumbers[stack[i - 2]]);
        }
    }
    printf("=== END CALL TRACE ===\n");
}
