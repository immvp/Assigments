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
    ins->rs1 = (memory_rd_w(mem, addr) >> 15) & 0x1F;
}

void rs2(struct instruction *ins, struct memory *mem, int addr) {
    ins->rs2 = (memory_rd_w(mem, addr) >> 20) & 0x1F;
}

void rd(struct instruction *ins, struct memory *mem, int addr) {
    ins->rd = (memory_rd_w(mem, addr) >> 7) & 0x1F;
}

void funct3(struct instruction *ins, struct memory *mem, int addr) {
    ins->funct3 = (memory_rd_w(mem, addr) >> 12) & 0x7;
}

void funct7(struct instruction *ins, struct memory *mem, int addr) {
    ins->funct7 = (memory_rd_w(mem, addr) >> 25) & 0x7F;
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
        // Skubber mest betydende bit til positionen længst til venstre
        firstBit = (unsigned)memory_rd_w(mem, addr) >> 31;
        // Starter ud med at arbejde med 0
        retval = 0;
        // Sætter de første 21 pladser i retval til samme værdi som firstBit
        for (int i = 0; i < 21; i++)
        {
            retval |= (firstBit << (11 + i));
        }

        // inst[30:20]
        // extract index 31 to 20
        extract = memory_rd_w(mem, addr) >> 20;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 11);
        // insert extracted value on index 10 to 0
        retval |= extract;
        ins->immediate = retval;
        break;
    case opcode_S:
        // Finds the first bit. Loops through index 11 to 31 and flips it.
        // inst[31]
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
        extract = memory_rd_w(mem, addr) & (1 << 7);
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
        firstBit = (unsigned)memory_rd_w(mem, addr) >> 31; // første bit til sidste plads
        retval = 0;
        for (int i = 0; i < 20; i++)
        {
            retval |= (firstBit << (12 + i));
        }

        //inst[7]
        // extract index 7
        extract = memory_rd_w(mem, addr) & (1 << 7);
        // move it to index 11
        extract = extract << 4;
        // insert extracted value on index 11
        retval |= (extract);

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

        // insert value 0 on index 0
        retval &= ~(1);
        // insert value into our struct
        ins->immediate = retval;
        break;
    case opcode_J:
        // Finds the first bit. Loops through index 12 to 31 and flips it.
        // inst[31]
        firstBit = (unsigned)memory_rd_w(mem, addr) >> 31; // første bit til sidste plads
        retval = 0;
        for (int i = 0; i < 12; i++)
        {
            retval |= (firstBit << (20 + i));
        }
        //inst[19:12]
        // extract index 19 to 12
        extract = memory_rd_w(mem, addr) >> 12;
        extract = extract << 12;
        extract = extract << 12;
        extract = extract >> 12;
        // insert extracted value on index 11
        retval |= extract;

        // inst[20]
        // extract index 20
        extract = memory_rd_w(mem, addr) & (1 << 20);
        // insert extract on index 11
        retval |= (extract >> 9);


        // inst[30:25]
        // extract index 31 to 25
        extract = memory_rd_w(mem, addr) >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst [24:21]
        extract = memory_rd_w(mem, addr) & (0xF << 21);
        extract = extract >> 20;
        retval |= extract;
        // insert value 0 on index 0
        retval |= 1;
        // insert value into our struct
        ins->immediate = retval;
        break;
    case opcode_U:
        // Sætter de 11 mindst betydende bits til 0
        retval = (unsigned)memory_rd_w(mem, addr) & 0x7FFFF800;
        ins->immediate = retval;
        break;
    default:
        break;
    }
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file)
{
    struct instruction ins;
    ins.opcode = opcode_R;
    calculate_immediate(&ins, mem, start_addr);
    rs1(&ins, mem, start_addr);
    rs2(&ins, mem, start_addr);
    rd(&ins, mem, start_addr);
    funct3(&ins, mem, start_addr);
    funct7(&ins, mem, start_addr);
    int data;
    switch (memory_rd_w(mem, start_addr) & 0x7F) // Read Opcode (6 bits from right to left)
    {
    case 0x0110011: // Opcode i R
        // check func3 values
        switch (ins.funct3 & 0x7000)
        {
        case 0x0:
            // Check func7 values
            switch (ins.funct7 & 0xFC000000)
            {
            case 0x0: // ADD
                data = memory_rd_w(mem, start_addr);
                data &= ~(0x1F << 7);
                data |= ((ins.rs1 + ins.rs2) << 7);
                memory_wr_w(mem, start_addr, data);
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }
    exit(0);
}