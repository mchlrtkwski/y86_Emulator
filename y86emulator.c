/******************************************************************************* 
 * Operating Systems
 * Summer 2016
 * Bryant
 * Project 3
 * y86-Assembler
 * 
 * Michael Rutkowski
 * 
 ******************************************************************************/

//Code Compiled and ran with the following text in a file for testing purposes:
/*******************************************************************************
 * 30f40001000030f50001000070240000000000000d000000c0000000000b000000a0000030f0
 * ffffff7fa00f30f2ffffff7fa02f803a00000000204550510800000050520c00000030f00000
 * 0000602121125016000000006060745700000090000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 0000000000000000000000000000000000000000000000000000000000000000000000000000
 * 000000000000000000000000
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//All opcode definitions
#define HALT 0x00
#define NOP 0x10
#define RRMOVL 0x20
#define CMOVLE 0x21
#define CMOVL 0x22
#define CMOVE 0x23
#define CMOVNE 0x24
#define CMOVGE 0x25
#define CMOVG 0x26
#define IRMOVL 0x30
#define RMMOVL 0x40
#define MRMOVL 0x50
#define ADDL 0x60
#define SUBL 0x61
#define ANDL 0x62
#define XORL 0x63
#define JMP 0x70
#define JLE 0x71
#define JL 0x72
#define JE 0x73
#define JNE 0x74
#define JGE 0x75
#define JG 0x76
#define CALL 0x80
#define RET 0x90
#define PUSHL 0xa0
#define POPL 0xb0

#define AOK 1
#define HLT 2
#define ADR 3
#define INS 4

//Global Variables
int mainMemory[1000];
int pc = 0;
int regis[8];
int zf = 0;
int sf = 0;
int of = 0;
int conditionCode = 1;
int isNotTaken = 0;
int lastInstIsAdd = 0;
int lastInstIsSub = 0;

/*******************************************************************************
 * Method: loadFile
 * params: char*
 * returns: int
 * Description: This method attempts to open a file for reading, if it
 * unsuccessful, a value of 0 is returned. If the file opening is successful,
 * the method then creates a 4 slot character array and sets the first 2 slots
 * to "0x". The next 2 slots are reserved to reading the two character. These 
 * character are reverted to a hexadecimal integer and added to the main memory
 * of the emulator.
 ******************************************************************************/
int loadFile(char * fileName) {

    FILE *file = fopen(fileName, "r");
    if (file != NULL) {
        char number[4];
        char endOfFile;
        int index = 0;
        while ((endOfFile = fgetc(file)) != EOF) {

            number[0] = '0';
            number[1] = 'x';
            number[2] = tolower(endOfFile);
            number[3] = tolower(fgetc(file));

            for (int a = 0; a < 4; a++) {
                number[a] = tolower(number[a]);
            }
            mainMemory[index] = (int) strtol(number, NULL, 0);
            index++;
        }
        fclose(file);

        for (int a = 0; a < 1000; a++) {
            printf("%x ", mainMemory[a]);
        }

        return 1;
    } else {
        return 0;
    }
}

/*******************************************************************************
 * Method: printMem
 * params: int, int
 * returns: int
 * Description: This method prints each byte of instrucitons to the screen.
 * 
 ******************************************************************************/
void printMem(int currentLocation, int bytesToProcess) {
    printf("0x%x\t", currentLocation);
    for (int a = currentLocation; a < (currentLocation + bytesToProcess); a++) {
        printf("%2x ", mainMemory[a]);
    }
    int numberOfTabs = 6 - bytesToProcess;
    for (int a = 0; a < numberOfTabs; a++) {
        printf("%2s ", " ");
    }
    printf("\t");

}

/*******************************************************************************
 * Method: printReg
 * params: n/a
 * returns: void
 * Description: This method simply prints the values of each register in. The 
 * method was created as a quick access to print each instruction after each
 * instruction.
 * 
 ******************************************************************************/
void printReg() {
    printf("\t\t");
    printf("%%eax:%4x ", regis[0]);
    printf("%%ecx:%4x ", regis[1]);
    printf("%%edx:%4x ", regis[2]);
    printf("%%ebx:%4x ", regis[3]);
    printf("%%esp:%4x ", regis[4]);
    printf("%%ebp:%4x ", regis[5]);
    printf("%%esi:%4x ", regis[6]);
    printf("%%edi:%4x ", regis[7]);
}

/*******************************************************************************
 * Method: findRegLabel
 * params: int
 * returns: void
 * Description: This method provides an easy way to print a register by label
 * according to their title rather than a value.
 * 
 ******************************************************************************/
void findRegLabel(int registerNumber) {

    if (registerNumber == 0) {
        printf("eax");
    } else if (registerNumber == 1) {
        printf("ecx");
    } else if (registerNumber == 2) {
        printf("edx");
    } else if (registerNumber == 3) {
        printf("ebx");
    } else if (registerNumber == 4) {
        printf("esp");
    } else if (registerNumber == 5) {
        printf("ebp");
    } else if (registerNumber == 6) {
        printf("esi");
    } else if (registerNumber == 7) {
        printf("edi");
    }
}

/*******************************************************************************
 * Method: resetFlags
 * params: n/a
 * return: void
 * Description: This method changes the value of all flags to 0.
 * 
 ******************************************************************************/
void resetFlags() {

    zf = 0;
    sf = 0;
    of = 0;

}

/*******************************************************************************
 * Method: setFlags
 * params: int
 * return: void
 * Description: This method takes the result of an operation and sets flags if
 * accordingly.
 ******************************************************************************/

void setFlags(int result, int oldValue, int operandNumber) {

    if (result == 0) {
        zf = 1;
    }
    if (result < 0) {
        sf = 1;
    }

    if (lastInstIsAdd) {
        if ((result < oldValue) && (operandNumber > 0)) {
            of = 1;
            lastInstIsAdd = 0;
        }
    }
    if (lastInstIsSub) {
        if ((result > oldValue) && (operandNumber > 0)) {
            of = 1;
            lastInstIsSub = 0;
        }
    }

}

/*******************************************************************************
 * Method: printFlags
 * params: n/a
 * returns:void
 * Description: This method is an easy-access method to print all set flags when
 * necessary.
 * 
 ******************************************************************************/
void printFlags() {

    if (zf) {
        printf("ZF ");
    }
    if (sf) {
        printf("SF ");
    }
    if (of) {
        printf("OF ");
    }
}

/*******************************************************************************
 * Method: printFlagsFinal
 * params: n/a
 * returns:void
 * Description: This method is an easy-access method to print all set flags when
 * at the end of the program.
 * 
 ******************************************************************************/
void printFlagsFinal() {
    printf("\tFlags that are Set upon Exit: ");
    if (zf) {
        printf("ZF ");
    }
    if (sf) {
        printf("SF ");
    }
    if (of) {
        printf("OF ");
    }
    printf(("\n"));
}

/*******************************************************************************
 * Method printMemoryDump
 * params: n/a
 * returns: void
 * Description: This method is a testing method created by me to check the 
 * results of memory after all instruction are ran. This method will be
 * commented out in main prior to project turn in.
 ******************************************************************************/
void printMemoryDump() {

    for (pc = 0; pc < 1000; pc++) {
        printf("0x%x\t", pc);
        printf("%x\n", mainMemory[pc]);
    }

}

/*******************************************************************************
 * Method checkValidAddress
 * params: int
 * returns: int
 * Description: This method is used to determine if an address is out of range 
 * for memory. If so, this method breaks the loop and exist with an invalid
 * address error
 ******************************************************************************/
int isValidAddress(int address) {

    if (address >= 1000) {
        return 0;
    } else {
        return 1;
    }

}

/*******************************************************************************
 * Method: runProgram
 * params: char*
 * returns: int
 * Description: This method iteratively cycles through the mainMemory array.
 * While doing so if a certain operation is encountered, the programes executes
 * the code and continues on. The method will terminate when a halt instruction
 * is encountered or if the program jumps out of bounds. There are 29 test
 * cases.
 *
 ******************************************************************************/
int runProgram() {
    if (conditionCode == AOK) {
        printf("\nAOK NORMAL OPERATION\n");
    }
    printf("%53s", " ");
    printReg();
    for (pc = 0; pc < 1000; pc++) {
        printf("PC: %x ", pc);
        printFlags();

        if (isNotTaken) {
            printf("(not taken)");
            isNotTaken = 0;

        }

        printf("\n");

        if (mainMemory[pc] == HALT) {

            //Print the full instruction
            printMem(pc, 1);
            printf("halt;                ");
            printReg();

            conditionCode = 2;

            pc = pc + 1;

            printf("PC: %x ", pc);
            if (isNotTaken) {
                printf("(not taken)");
                isNotTaken = 0;

            }
            printf("\n");
            printf("HLT HALT INSTRUCTION ENCOUNTERED");
            
            printFlagsFinal();

            break;

            //##################################################################
        } else if (mainMemory[pc] == NOP) {

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == RRMOVL) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Print Instruction Details
            printf("rrmovl ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Swap
            regis[regB] = regis[regA];

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVLE) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if ((zf == 1) || (sf == 1)) {

                //Swap
                regis[regB] = regis[regA];
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmovle ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVL) {
            int followInstruct = 0;

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if (sf == 1) {
                //Swap
                regis[regB] = regis[regA];

                followInstruct = 1;
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmovl ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVE) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if (zf == 1) {
                //Swap
                regis[regB] = regis[regA];
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmove ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVNE) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if ((zf != 1)) {
                //Swap
                regis[regB] = regis[regA];
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmovne ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVGE) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if ((zf = 1) || (sf != 1)) {
                //Swap
                regis[regB] = regis[regA];
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmovge ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();


            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CMOVG) {

            //Print the full instruction
            printMem(pc, 2);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            if ((zf != 1) && (sf != 1)) {
                //Swap
                regis[regB] = regis[regA];
            } else {
                isNotTaken = 1;
            }

            //Print Instruction Details
            printf("cmovg ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%6s", " ");

            //Print Registers
            printReg();

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == IRMOVL) {

            //Print the full instruction
            printMem(pc, 6);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Book keeping
            int registerToChange = (mainMemory[pc] & 0x0f);
            int valueToStore = 0;

            //Concatenate the value of the bytes in little Endianess.
            for (int a = 4; a >= 1; a--) {

                valueToStore = valueToStore << 8;

                valueToStore = valueToStore + mainMemory[pc + a];

            }
            regis[registerToChange] = valueToStore;

            //Print Instruction
            printf("%6s", "irmovl ");
            findRegLabel(registerToChange);
            printf(", ");
            printf("%-8x ", valueToStore);

            //Print the Registers
            printReg();

            //Advance the PC counter to the end of the instruction
            pc = pc + 4;

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == RMMOVL) {

            //Print the full instruction
            printMem(pc, 6);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Determine Value that must be the offset
            int valueToAdd = 0;
            for (int a = 4; a >= 1; a--) {

                valueToAdd = valueToAdd << 8;

                valueToAdd = valueToAdd + mainMemory[pc + a];

            }

            //Print the Instruction Details
            printf("rmmovl %x(", valueToAdd);
            findRegLabel(regA);
            printf("), ");
            findRegLabel(regB);
            printf("%3s", " ");

            //Move the Value in Register into Memory
            mainMemory[regis[regA] + valueToAdd] = regis[regB];


            //Print the Registers
            printReg();

            pc = pc + 4;

            resetFlags();


            //##################################################################
        } else if (mainMemory[pc] == MRMOVL) {

            //Print the full instruction
            printMem(pc, 6);

            //Advance PC counter to find the proper register
            pc = pc + 1;

            //Find RegA
            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Determine Value that must be the offset
            int valueToAdd = 0;
            for (int a = 4; a >= 1; a--) {

                valueToAdd = valueToAdd << 8;

                valueToAdd = valueToAdd + mainMemory[pc + a];

            }

            //Print the Instruction Details
            printf("mrmovl %x(", valueToAdd);
            findRegLabel(regA);
            printf("), ");
            findRegLabel(regB);
            printf("%3s", " ");

            //Move the Value in Memory at the index of RegA into RegB
            regis[regB] = mainMemory[regis[regA] + valueToAdd];

            //Print the Registers
            printReg();

            pc = pc + 4;

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == ADDL) {

            //Print the full instruction
            printMem(pc, 2);

            pc = pc + 1;

            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Print Instruction Detail
            printf("add ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%9s", " ");

            //Store result of 'add' into RegB
            int oldValue = regis[regB];
            int operandNumber = regis[regA];
            regis[regB] = regis[regB] + regis[regA];


            //Set Condition Code
            lastInstIsAdd = 1;
            setFlags(regis[regB], oldValue, operandNumber);

            //Print the Registers
            printReg();

            //##################################################################
        } else if (mainMemory[pc] == SUBL) {

            //Print the full instruction
            printMem(pc, 2);

            pc = pc + 1;

            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Print Instruction Detail
            printf("sub ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%9s", " ");

            //Store result of 'and' into RegB
            int oldValue = regis[regB];
            int operandNumber = regis[regA];
            regis[regB] = regis[regB] - regis[regA];

            //Set Condition Code
            lastInstIsSub = 1;
            setFlags(regis[regB], oldValue, operandNumber);

            //Print the Registers
            printReg();

            //##################################################################
        } else if (mainMemory[pc] == ANDL) {

            //Print the full instruction
            printMem(pc, 2);

            pc = pc + 1;

            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Print Instruction Detail
            printf("and ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%9s", " ");

            //Store result of 'and' into RegB
            regis[regB] = regis[regA] & regis[regB];

            //Set Condition Code
            setFlags(regis[regB], 0, 0);

            //Print the Registers
            printReg();

            //##################################################################
        } else if (mainMemory[pc] == XORL) {

            //Print the full instruction
            printMem(pc, 2);

            pc = pc + 1;

            int regA = (mainMemory[pc] & 0xf0);
            regA = regA >> 4;

            //Find RegB
            int regB = (mainMemory[pc] & 0x0f);

            //Print Instruction Detail
            printf("xor ");
            findRegLabel(regA);
            printf(", ");
            findRegLabel(regB);
            printf("%9s", " ");

            //Store result of 'and' into RegB
            regis[regB] = regis[regA] ^ regis[regB];

            //Set Condition Code
            setFlags(regis[regB], 0, 0);

            //Print the Registers
            printReg();

            //##################################################################
        } else if (mainMemory[pc] == JMP) {

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jmp %x (PC = %x)%4s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            //Subtract 1 from the Jump to account for the for-loop increment
            pc = valueToGoto - 1;

            //Check to see if the jumped to address is within range.
            if (!isValidAddress(valueToGoto)) {
                printf("\nADR INVALID ADDRESS ENCOUNTERED");
                printFlagsFinal();
                break;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == JLE) {
            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jle %x (PC = %x)%4s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if ((zf == 1) || (sf == 1)) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                //Check to see if the jumped to address is within range.
                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }
            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == JL) {

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jl %x (PC = %x)%5s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if (sf == 1) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                //Check to see if the jumped to address is within range.
                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }
            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == JE) {

            int followInstruct = 0;

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("je %x (PC = %x)%5s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if (zf == 1) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                //Check to see if the jumped to address is within range.
                followInstruct = 1;
                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }
            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();


            //##################################################################
        } else if (mainMemory[pc] == JNE) {

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jne %x (PC = %x)%4s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if ((zf != 1)) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }
            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == JGE) {

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jge %x (PC = %x)%4s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if ((zf = 1) || (sf != 1)) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }
            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == JG) {

            //Print the full instruction
            printMem(pc, 5);

            //Increment PC to find Location of Jump
            // pc = pc + 1;

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Action
            printf("jg %x (PC = %x)%5s ", valueToGoto, valueToGoto, " ");

            //Print Registers
            printReg();

            if ((zf != 1) && (sf != 1)) {
                //Subtract 1 from the Jump to account for the for-loop increment
                pc = valueToGoto - 1;

                //Check to see if the jumped to address is within range.
                if (!isValidAddress(valueToGoto)) {
                    printf("\nADR INVALID ADDRESS ENCOUNTERED");
                    printFlagsFinal();
                    break;
                }

            } else {
                isNotTaken = 1;
                pc = pc + 4;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == CALL) {

            //Print the full instruction
            printMem(pc, 5);

            //Determine Value in little Endianess
            int valueToGoto = 0;
            for (int a = 4; a >= 1; a--) {

                valueToGoto = valueToGoto << 8;

                valueToGoto = valueToGoto + mainMemory[pc + a];
            }

            //Print Instruction Details
            printf("call %-16x", valueToGoto);

            //Store the Return address on %esp
            pc = pc + 5;
            mainMemory[regis[4]] = pc;

            //Decrement Stack
            regis[4] = regis[4] - 4;

            //Set the PC to the Call value -1 to account for the for loop
            pc = valueToGoto - 1;

            //Print Registers
            printReg();

            //Check to see if the jumped to address is within range.
            if (!isValidAddress(valueToGoto)) {
                printf("\nADR INVALID ADDRESS ENCOUNTERED");
                printFlagsFinal();
                break;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == RET) {

            //Print the full instruction
            printMem(pc, 1);

            //increment %esp
            regis[4] = regis[4] + 4;

            //Assign the register value %esp
            pc = mainMemory[regis[4]] - 1;

            //Print Action
            printf("ret %-16s ", " ");

            //Print Registers
            printReg();

            //Check to see if the jumped to address is within range.
            if (!isValidAddress(pc)) {
                printf("\nADR INVALID ADDRESS ENCOUNTERED");
                printFlagsFinal();
                break;
            }

            resetFlags();

            //##################################################################
        } else if (mainMemory[pc] == PUSHL) {

            //Print the full instruction
            printMem(pc, 2);

            //Increment PC to find Register Location and use bitwise math
            pc = pc + 1;
            int registerToPush = (mainMemory[pc] & 0xf0);
            registerToPush = registerToPush >> 4;

            //Assign the register value %esp
            mainMemory[regis[4]] = regis[registerToPush];

            //Print Action
            printf("pushl %-14x ", regis[registerToPush]);

            //Decrement %esp
            regis[4] = regis[4] - 4;

            //Print Registers
            printReg();

            resetFlags();


            //##################################################################
        } else if (mainMemory[pc] == POPL) {

            //Print the full instruction
            printMem(pc, 2);

            //Increment PC to find Register Location and use bitwise math
            pc = pc + 1;
            int registerToPop = (mainMemory[pc] & 0xf0);
            registerToPop = registerToPop >> 4;

            //increment %esp
            regis[4] = regis[4] + 4;

            //Assign the register value %esp
            regis[registerToPop] = mainMemory[regis[4]];

            //Print Action
            printf("popl %-15x ", regis[registerToPop]);

            //Print Registers
            printReg();

            resetFlags();

        } else {
            conditionCode = 2;
            printf("INS INVALID INSTRUCTION ENCOUNTERED");
            printFlagsFinal();
            break;


        }
    }
    return 1;
}

/*******************************************************************************
 * Method: main
 * params: int, char** argv
 * returns: int
 * Description: This method takes the arguments and runs it through the
 * appropriate method calls. 
 ******************************************************************************/
int main(int argc, char** argv) {

    for (int i = 0; i < 1000; i++) {
        mainMemory[i] = 0;
    }

    for (int i = 0; i < 8; i++) {
        regis[i] = 0;
    }

    if (argc != 2) {
        printf("Enter a single Binary file\n");
    } else {
        char * fileName = (char *) argv[1];
        if (loadFile(fileName)) {
            printf("\n\nfile found and parsed into memory...\n");
            runProgram();
            //printMemoryDump();
        } else {
            printf("Error parsing file...");
        }

    }

    return (EXIT_SUCCESS);
}

