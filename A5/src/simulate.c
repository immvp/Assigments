#include "read_exec.h"
#include "assembly.h"
#include "simulate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum opcodes {
    opcode_R = 0x73,
    opcode_R_1 = 0x0,
    opcode_I_1 = 0x13,
    opcode_I_2 = 0x3,
    opcode_I_3 = 0x67,
    opcode_I_4 = 0x73,
    opcode_S = 0x23,
    opcode_B = 0x63,
    opcode_J = 0x6F,
    opcode_U = 0x37
};

struct instruction {
    int opcode;
    int rd;
    int funct3;
    int rs1;
    int rs2;
    int funct7;
    int immediate;
} instruction;

void printBinary(unsigned int num) {
    if (num > 1) {
        printBinary(num >> 1); // Recursively call function to print remaining bits
    }
    else {
        printf("\n");
    }
    printf("%d", num & 1); // Print the least significant bit
}

// Step 1: Lav funktioner til at indsætte rd, funct3, rs1, rs2 v/
// Step 2: Lav funktioner der udregner immediate for de forskellige typer (R, I, S, U)
// Step 3: Udfyld switch statements med de forskellige assembly instruktioner

void rs1(struct instruction *ins, struct memory *mem, int addr) {
    ins->rs1 = (memory_rd_w(mem, addr) >> 14) & 0x1F;
}

void rs2(struct instruction *ins, struct memory *mem, int addr) {
    ins->rs2 = (memory_rd_w(mem, addr) >> 19) & 0x1F;
}

void rd(struct instruction *ins, struct memory *mem, int addr) {
    ins->rd = (memory_rd_w(mem, addr) >> 6) & 0x1F;
}

void funct3(struct instruction *ins, struct memory *mem, int addr) {
    ins->funct3 = (memory_rd_w(mem, addr) >> 11) & 0x7;
}

void funct7(struct instruction *ins, struct memory *mem, int addr) {
    ins->funct7 = (memory_rd_w(mem, addr) >> 24) & 0x7F;
}

void calculate_immediate(struct instruction *ins, struct memory *mem, int addr) {
    unsigned int testBit = 0xFFFFFFFF;
    unsigned int firstBit;
    unsigned int retval = 0;
    unsigned int extract;
    switch (ins->opcode)
    {
    case opcode_I_1:
    case opcode_I_2:
    case opcode_I_3:
    case opcode_I_4:
        // unsigned firstBit = (unsigned)memory_rd_w(mem, addr) >> 31; // flytter den første til sidste plads
        //     for (int i = 0; i < 20; ++i) {
        //         ins->immediate = (unsigned)memory_rd_w(mem, addr) | firstBit << (19 - i); // sætter de første pladser til firstBit (mangler nok plus fremfor minus)
        //     }
        // ins->immediate = (memory_rd_w(mem, addr) & 0x7FF00000) | ((memory_rd_w(mem, addr) >> 19) & 0x7FF);
        break;
    case opcode_S:
        // Finds the first bit. Loops through index 11 to 31 and flips it.
        // inst[31]
        testBit = 0xFFFFFFFF;
        firstBit = (unsigned)memory_rd_w(mem, addr) >> 31; // første bit til sidste plads
        retval = 0;
        for (int i = 0; i < 21; i++)
        {
            retval |= (firstBit << (11 + i));
        }

        // inst[30:25]
        // extract index 31 to 25
        extract = memory_rd_w(mem, addr) >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst[11:8]
        // extract index 11 to 8
        extract = memory_rd_w(mem, addr) << 20;
        extract = memory_rd_w(mem, addr) >> 28;
        // insert extracted value on 4 to 1
        retval |= (extract << 1);

        //inst[7]
        // extract index 7
        extract = testBit & (1 << 7);
        // move it to index 0
        extract = extract >> 7;
        // insert extracted value on index 0
        retval |= (extract);

        // insert value into our struct
        ins->immediate = retval;
        break;
    case opcode_B:
        // Finds the first bit. Loops through index 12 to 31 and flips it.
        // inst[31]
        testBit = 0xFFFFFFFF;
        firstBit = (unsigned)testBit >> 31; // første bit til sidste plads
        retval = 0;
        for (int i = 0; i < 20; i++)
        {
            retval |= (firstBit << (12 + i));
        }

        //inst[7]
        // extract index 7
        extract = testBit & (1 << 7);
        // move it to index 11
        extract = extract << 4;
        // insert extracted value on index 11
        retval |= (extract);

        // inst[30:25]
        // extract index 31 to 25
        extract = testBit >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst[11:8]
        // extract index 11 to 8
        extract = testBit << 20;
        extract = testBit >> 28;
        // insert extracted value on 4 to 1
        retval |= (extract << 1);

        // insert value 0 on index 0
        retval &= ~(1);
        // insert value into our struct
        ins->immediate = retval;
        printBinary(ins->immediate);
        break;
    case opcode_J:
        break;
    case opcode_U:
        // unsigned bit = 
        // ins->immediate = memory_rd_w(mem, addr);
        // ins->immediate &= 0xFFF;
        break;
    default:
        break;
    }
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file)
{
    struct instruction ins;
    ins.opcode = opcode_B;
    calculate_immediate(&ins, mem, start_addr);
    printf("\n");
    // switch (memory_rd_w(mem, start_addr) & 0x7F) // Read Opcode (6 bits from right to left)
    // {
    // case 0x0110011: // Opcode i R
    //     // check func3 values
    //     switch (memory_rd_w(mem, start_addr) & 0x7000)
    //     {
    //     case 0x0:
    //         // Check func7 values
    //         switch (memory_rd_w(mem, start_addr) & 0xFC000000)
    //         {
    //         case 0x0: // ADD
    //             int rd = memory_rd_w(mem, start_addr) & 0xF80;
    //             int temp = memory
    //             break;

    //         default:
    //             break;
    //         }
    //         break;

    //     default:
    //         break;
    //     }
    //     break;
    // default:
    //     break;
    // }
    exit(0);
}