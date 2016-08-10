#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * INDIGO COMPILER V1.0.1
 * INDIGO IS A LANGUAGE WHICH UTILIZES HEX VALUES FOR ALL INSTUCTIONS.
 * WRITTEN BY JOSEPH BOYLE (joeb3219).
 * Indigo uses a stack to process all instructions, which can be read from a text file.
 * The Indigo Compiler uses 8192 memory registers to store various data.
 * Each register has a marker which indicates whether or not it is a "special" value - some values stored in registers are characters, IF statements, etc.
 * By marking a register as "special", we are able to differentiate different types of data in the system. For example, when PRINT is called, the compiler checks if the identifier "1" is used,
 * in which case it uses the character represented by the value of the register.
 *
**/

static int instructions[1024 * 4] = {-1}; //A list of instructions to process in the application.
static int stack[1024 * 2] = {-1}; //The stack which is used to push values to other segments of code.
static int tail = -1; //This is the last used position on the stack (eg, after one insert, tail will be set to 0).
static int registers[1024 * 8] = {-1}; //The registers which data may be set/retrieved to/from during program execution. IE: Memory.
static int markers[1024 * 8] = {-1}; //Used to tell us what kind of data a value in a register actually represents (eg: setting
static int specialIdentifierAtStackPosition[1024];
static int functions[128][2] = {-1}; //A list of functions stored by the program. functions[][0] contains the instruction number, and [][1] contains the number of args.

const int _IDENTIFIER_FUNCTION_DECLARE = -132; //A flag used to identify a function declaration, which we ignore for now.
const int _IDENTIFIER_FUNCTION = -131; //A flag that to identify when we were in a function call.
const int _IDENTIFIER_FAILED_IF = -130; //A flag that, if pushed the the stack, will cause the program to cease execution of instructions until an END statement is met.
const int _IDENTIFIER_GOOD_IF = -129; //A flag used to identify IF conditions which the program is to go inside of.
const int _IDENTIFIER_CHAR = -128; //A flag used to identify that the register represents a character.

int main(int argc, char *argv[]){
    FILE *file;
    int k = 0;

    printf("Hello world! I am a Virtual Machine!\n");

    if(argc < 2) error("Insufficient number of arguments; expecting filename.");

    file = fopen(argv[1], "r");

    k = processFile(file, instructions, 0);

    fclose(file);

    interpret(k);

    return 0;
}

//Processes a file, returning the total number of instructions extracted from the file.
int processFile(FILE* file, int* theseInstructions, int startRegister){
    char line[4096];
    char processedLine[4096];
    char substring[11]; //Allows for an instruction with a maximum value of 0xffffffff plus an \0 character.
    int val = 0, i = 0, j = 0, start = 0, k = startRegister;

    while (fgets(line, sizeof(line), file)) {
        j = 0;
        for(i = 0; i < sizeof(line); i ++){ //Copy over non-space, non EOL characters to be processed.
            if(line[i] == '/' || line[i] == '\n'){
                i = sizeof(line);
                continue;
            } else if(line[i] != ' '){
                processedLine[j] = line[i];
                j ++;
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
            theseInstructions[k] = val; //Add instruction to the list
            k ++;
            start = i - 1;
        }
        memset(processedLine, 0, sizeof(processedLine)); //Clear out the line so we don't reread larger instructions in the next cycle.
    }
    return k;
}

void interpret(int numInstructions){
    int i, a, b = 0;
    for(i = 0; i < numInstructions; i ++){
        //If the current item on the stack is in fact an if statement which we aren't entering, only consider other IF/END statements
        if(peek() == _IDENTIFIER_FAILED_IF && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FAILED_IF){
            if(instructions[i] == 0x13){
                push(_IDENTIFIER_FAILED_IF);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FAILED_IF;
            }else if(instructions[i] == 0x08) pop(); //Pops off the IF call.
            continue;
        }else if(peek() == _IDENTIFIER_FUNCTION_DECLARE && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION_DECLARE){
            if(instructions[i] == 0x08) pop(); //Pops off the FUNCTION_DECLARE call.
            continue;
        }
        switch(instructions[i]){
            case 0x00: //LITERAL: literal a: Pushes a to the stack
                i ++;
                push(instructions[i]);
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
                while(!( (peek() == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION)
                        || (peek() == _IDENTIFIER_FAILED_IF && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FAILED_IF)
                        || (peek() == _IDENTIFIER_GOOD_IF && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_GOOD_IF) ) ) {
                    if(tail == -1) break;
                    pop(); //Until we get to a condition we want, let's keep popping values.
                }
                if(peek() == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION){
                    pop();
                    i = pop(); //Return to calling instruction.
                }else pop();
                break;
            case 0x09: //GET_REGISTER: a GET_REGISTER: Pushes the value of register a to the stack.
                a = pop();
                push(registers[a]);
                break;
            case 0x0a: //SET_REGISTER: a b SET_REGISTER: Sets the value of register a to b.
                b = pop();
                a = pop();
                registers[a] = b;
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
                a = pop();
                if(a == 0){
                    push(_IDENTIFIER_FAILED_IF);
                    specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FAILED_IF;
                }
                else{
                    push(_IDENTIFIER_GOOD_IF);
                    specialIdentifierAtStackPosition[tail] = _IDENTIFIER_GOOD_IF;
                }
                break;
            case 0x14: //SET_MARKET: a SET_MARKET: Sets a marker at position a to the current instruction.
                a = pop();
                markers[a] = i;
                break;
            case 0x15: //JUMP_TO: a JUMP_TO: Jump to the instruction specified by marker[a].
                a = pop();
                i = markers[a];
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
            case 0x1d: //READ_FILE: a b READ_FILE: reads in instructions from file named a.txt starting at register b.
                b = pop();
                a = pop();

                char fileName[12];
                sprintf(fileName, "%d.txt", a);

                FILE *file = fopen(fileName, "r");

                processFile(file, registers, b);

                fclose(file);
                break;
            case 0x1e: //FUNCTION_DECLARE: a b FUNCTION_DECLARE: Creates a function with ID a, which takes the last b parameters pushed to the stack.
                b = pop();
                a = pop();
                functions[a][0] = i;
                functions[a][1] = b;
                push(_IDENTIFIER_FUNCTION_DECLARE);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FUNCTION_DECLARE;
                break;
            case 0x1f: //FUNCTION_JUMP: a FUNCTION_JUMP: Jumps to a function with the id a.
                a = pop();
                push(i); //Push the current instruction number for use later.
                push(_IDENTIFIER_FUNCTION);
                specialIdentifierAtStackPosition[tail] = _IDENTIFIER_FUNCTION;
                i = functions[a][0];
                /*for(b = functions[a][0]; b > 0; b --){
                    pop(); //Pops off the last b arguments for use by the function.
                }*/
                break;
            case 0x20: //RETURN: a RETURN: Pushes a to the stack after function execution ends.
                b = specialIdentifierAtStackPosition[tail];
                a = pop(); //Save the value which we wish to remain on the stack
                while(!( (peek() == _IDENTIFIER_FUNCTION && specialIdentifierAtStackPosition[tail] == _IDENTIFIER_FUNCTION) ) ){
                    if(tail == -1) break;
                    pop(); //Until we get to a condition we want, let's keep popping values.
                }
                pop(); // Remove the function call from the stack
                i = pop(); //Return to calling instruction.
                push(a); // Push a back onto the stack
                specialIdentifierAtStackPosition[tail] = b;
                break;
        }
    }
}

//Returns the object that would next be popped from the stack without actually popping it.
int peek(){
    if(tail < 0) return -1;
    return stack[tail];
}

//Pops an object from the stack.
//Also resets the special registers back to zero.
int pop(){
    if(tail < 0){
        error("Attempting to pop an empty stack.");
        return -1;
    }
    specialIdentifierAtStackPosition[tail] = 0; //Reset back to being not special
    int val = stack[tail];
    stack[tail] = -1;
    tail --;
    return val;
}

//Pushes an element onto the top of the stack.
void push(int val){
    tail ++;
    stack[tail] = val;
}

//If an error if ever thrown, stop execution of the program.
void error(char* message){
    printf("[COMPILER] ERROR: %s\n", message);
    exit(1);
}
