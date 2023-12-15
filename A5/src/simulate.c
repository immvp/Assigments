#include "read_exec.h"
#include "assembly.h"
#include "simulate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum opcodes
{
    opcode_R = 0x33,
    opcode_I_1 = 0x13,
    opcode_I_2 = 0x3,
    opcode_I_3 = 0x67,
    opcode_I_4 = 0x73,
    opcode_S = 0x23,
    opcode_B = 0x63,
    opcode_J = 0x6F,
    opcode_U_1 = 0x37,
    opcode_U_2 = 0x17
};

struct instruction
{
    int opcode;
    int *rd;
    int funct3;
    int rs1;
    int rs2;
    int funct7;
    int immediate;
} instruction;

__int32_t reg[32];

void printBinary(unsigned int num)
{
    if (num > 1)
    {
        printBinary(num >> 1); // Recursively call function to print remaining bits
    }
    else
    {
        printf("\n");
    }
    printf("%d", num & 1); // Print the least significant bit
}

void rs1(struct instruction *ins, struct memory *mem, int addr)
{
    ins->rs1 = reg[(memory_rd_w(mem, addr) >> 15) & 0x1F];
}

void rs2(struct instruction *ins, struct memory *mem, int addr)
{
    ins->rs2 = reg[(memory_rd_w(mem, addr) >> 20) & 0x1F];
}

void rd(struct instruction *ins, struct memory *mem, int addr)
{
    ins->rd = &reg[(memory_rd_w(mem, addr) >> 7) & 0x1F];
}

void funct3(struct instruction *ins, struct memory *mem, int addr)
{
    ins->funct3 = (memory_rd_w(mem, addr) >> 12) & 0x7;
}

void funct7(struct instruction *ins, struct memory *mem, int addr)
{
    ins->funct7 = (memory_rd_w(mem, addr) >> 25) & 0x7F;
}

void insert_data(int *data, int insertValue)
{
    *data &= ~(0x1F << 7);
    *data |= ((insertValue) << 7);
}

void increment_PC(int *data)
{
    *data += 4;
}

void calculate_immediate(struct instruction *ins, struct memory *mem, int addr)
{
    // printf("[addr: %d]\n", addr);
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
        extract = (unsigned)memory_rd_w(mem, addr) >> 20;
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
        extract = (unsigned)memory_rd_w(mem, addr) >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst[11:8]
        // extract index 11 to 8
        extract = ((unsigned)memory_rd_w(mem, addr) >> 8) & 0xF;
        // insert extracted value on 4 to 1
        retval |= (extract << 1);

        // inst[7]
        //  extract index 7
        extract = (unsigned)memory_rd_w(mem, addr) & (1 << 7);
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

        // inst[7]
        //  extract index 7
        extract = (unsigned)memory_rd_w(mem, addr) & (1 << 7);
        // move it to index 11
        extract = extract << 4;
        // insert extracted value on index 11
        retval |= (extract);

        // inst[30:25]
        // extract index 31 to 25
        extract = (unsigned)memory_rd_w(mem, addr) >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst[11:8]
        // extract index 11 to 8
        extract = (unsigned)memory_rd_w(mem, addr) << 20;
        extract = (unsigned)memory_rd_w(mem, addr) >> 28;
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
        // inst[19:12]
        //  extract index 19 to 12
        extract = (unsigned)memory_rd_w(mem, addr) >> 12;
        extract = extract << 12;
        extract = extract << 12;
        extract = extract >> 12;
        // insert extracted value on index 11
        retval |= extract;

        // inst[20]
        // extract index 20
        extract = (unsigned)memory_rd_w(mem, addr) & (1 << 20);
        // insert extract on index 11
        retval |= (extract >> 9);

        // inst[30:25]
        // extract index 31 to 25
        extract = (unsigned)memory_rd_w(mem, addr) >> 25;
        // flip index 31 to 0, we dont care about it
        extract &= ~(1 << 6);
        // insert extracted value on index 10 to 5
        retval |= (extract << 5);

        // inst [24:21]
        extract = (unsigned)memory_rd_w(mem, addr) & (0xF << 21);
        extract = extract >> 20;
        retval |= extract;
        // insert value 0 on index 0
        retval |= 1;
        // insert value into our struct
        ins->immediate = retval;
        break;
    case opcode_U_1:
    case opcode_U_2:
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
    int PC = start_addr;
    while (1)
    {
        ins.opcode = memory_rd_w(mem, PC) & 0x7F;
        calculate_immediate(&ins, mem, PC);
        rs1(&ins, mem, PC);
        rs2(&ins, mem, PC);
        rd(&ins, mem, PC);
        funct3(&ins, mem, PC);
        funct7(&ins, mem, PC);
        int data;
        int insertdata;
        printf("Opcode: %x | rs1: %d | rs2: %d | rd: %d | funct3: %d | funct7: %d\n", ins.opcode,
                ins.rs1, ins.rs2, *ins.rd, ins.funct3, ins.funct7);
        switch (ins.opcode) // Read Opcode (6 bits from right to left)
        {
        case opcode_R: // Opcode i R
            // check func3 values
            switch (ins.funct3)
            {
            case 0x0:
                if (ins.funct7 == 0x00)
                { // ADD
                    insertdata = ins.rs1 + ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x20)
                { // SUB
                    insertdata = ins.rs1 - ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01) // mul
                {
                    insertdata = ins.rs1 * ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x2:
                if (ins.funct7 == 0x01)
                { // mulsu
                    __int64_t tempo = (__int32_t)ins.rs1 * (__uint32_t)ins.rs2;
                    insertdata = (__int32_t)(tempo >> 32);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01) // slt
                {
                    insertdata = (ins.rs1 < ins.rs2) ? 1 : 0;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x4: // XOR
                if (ins.funct7 == 0x00)
                {
                    insertdata = ins.rs1 ^ ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01)
                {
                    if (ins.rs2 != 0)
                    {
                        insertdata = ins.rs1 / ins.rs2;
                        *ins.rd = insertdata;
                        increment_PC(&PC);
                    }
                }
                break;
            case 0x6:
                if (ins.funct7 == 0x00) // OR
                {
                    insertdata = ins.rs1 | ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x00) // rem
                {
                    insertdata = ins.rs1 % ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x7:
                if (ins.funct7 == 0x00) // AND
                {
                    insertdata = ins.rs1 & ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01) // remu
                {
                    insertdata = (unsigned)ins.rs1 % (unsigned)ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x1:
                if (ins.funct7 == 0x00) // sll
                {
                    insertdata = ins.rs1 << ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01)
                { // mulh
                    __int64_t temp = (__int64_t)ins.rs1 * (__int64_t)ins.rs2;
                    insertdata = (__int32_t)(temp >> 32);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x5:
                if (ins.funct7 == 0x00)
                { // srl
                    insertdata = (unsigned)ins.rs1 >> (unsigned)ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x5)
                { // sra
                    insertdata = ins.rs1 >> ins.rs2;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x01)
                {
                    if (ins.rs2 != 0)
                    {
                        insertdata = (__uint32_t)ins.rs1 / (__uint32_t)ins.rs2;
                        *ins.rd = insertdata;
                        increment_PC(&PC);
                    }
                }
                break;
            case 0x3: // sltu
                if (ins.funct7 == 0x01)
                {
                    __uint64_t tempo = (__uint32_t)ins.rs1 * (__uint32_t)ins.rs2;
                    insertdata = (__uint32_t)(tempo >> 32);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (ins.funct7 == 0x0)
                {
                    insertdata = ((unsigned)ins.rs1 < (unsigned)ins.rs2) ? 1 : 0;
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;

            default:
                break;
            }
            break;
        case opcode_I_1: // Opcode i R
            // check func3 values
            switch (ins.funct3)
            {
            case 0x0: // addi
                insertdata = ins.rs1 + ins.immediate;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x4: // xori
                insertdata = ins.rs1 ^ ins.immediate;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x6: // ori
                insertdata = ins.rs1 | ins.immediate;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x7: // andi
                insertdata = ins.rs1 & ins.immediate;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x1: // slli
                if (((ins.immediate >> 5) & 0x7F) == 0x00)
                {
                    insertdata = ins.rs1 << (ins.immediate & 0xF);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x5:
                if (((ins.immediate >> 5) & 0x7F) == 0x00)
                { // srli
                    insertdata = (unsigned)ins.rs1 >> (unsigned)(ins.immediate & 0xF);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                else if (((ins.immediate >> 5) & 0x7F) == 0x02)
                { // srali
                    insertdata = ins.rs1 >> (ins.immediate & 0xF);
                    *ins.rd = insertdata;
                    increment_PC(&PC);
                }
                break;
            case 0x2: // slti
                insertdata = (ins.rs1 < ins.immediate) ? 1 : 0;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x3: // sltiu
                insertdata = ((unsigned)ins.rs1 < (unsigned)ins.immediate) ? 1 : 0;
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;

            default:
                break;
            }
            break;
        case opcode_I_2:
            switch (ins.funct3)
            {
            case 0x0: // lb
                insertdata = memory_rd_b(mem, PC + ins.rs1 + ins.immediate);
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x1: // lh
                insertdata = memory_rd_h(mem, PC + ins.rs1 + ins.immediate);
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x2: // lw
                insertdata = memory_rd_w(mem, PC + ins.rs1 + ins.immediate);
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x4: // lbu
                insertdata = memory_rd_b(mem, PC + ((unsigned)ins.rs1 + (unsigned)ins.immediate));
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;
            case 0x5: // lhu
                insertdata = memory_rd_h(mem, PC + ((unsigned)ins.rs1 + (unsigned)ins.immediate));
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;

            default:
                break;
            }
            break;
        case opcode_S:
            switch (ins.funct3)
            {
            case 0x0: // sb
                memory_wr_b(mem, PC + ins.rs1 + ins.immediate, ins.rs2);
                increment_PC(&PC);
                break;  
            case 0x1: // sh
                memory_wr_h(mem, PC + ins.rs1 + ins.immediate, ins.rs2);
                increment_PC(&PC);
                break;
            case 0x2: // sw
                memory_wr_w(mem, PC + ins.rs1 + ins.immediate, ins.rs2);
                increment_PC(&PC);
                break;
            default:
                break;
            }
            break;
        case opcode_B:
            switch (ins.funct3)
            {
            case 0x0: // beq
                if (ins.rs1 == ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;
            case 0x1: // bne
                if (ins.rs1 != ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;
            case 0x4: // blt
                if (ins.rs1 < ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;
            case 0x5: // bge
                if (ins.rs1 >= ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;
            case 0x6: // bltu
                if ((unsigned)ins.rs1 < (unsigned)ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;
            case 0x7: // bgeu
                if ((unsigned)ins.rs1 >= (unsigned)ins.rs2)
                {
                    PC += ins.immediate;
                }
                else
                {
                    increment_PC(&PC);
                }
                break;

            default:
                break;
            }
            break;
        case opcode_J: // jal
            insertdata = PC + 4;
            *ins.rd = insertdata;
            PC += ins.immediate;
            break;
        case opcode_I_3: // jalr
            switch (ins.funct3)
            {
            case 0x0:
                insertdata = PC + 4;
                *ins.rd = insertdata;
                PC = ins.rs1 + ins.immediate;
                break;

            default:
                break;
            }
            break;
        case opcode_U_1: // lui
            insertdata = ins.immediate << 12;
            *ins.rd = insertdata;
            increment_PC(&PC);
            break;
        case opcode_U_2: // auipc
            switch (ins.funct3)
            {
            case 0x0:
                insertdata = PC + (ins.immediate << 12);
                *ins.rd = insertdata;
                increment_PC(&PC);
                break;

            default:
                break;
            }
            break;
        case opcode_I_4:
            switch (ins.funct3)
            {
            case 0x0: // ecall
                if (ins.immediate == 0x0)
                {
                    printf("\nExiting...");
                    exit(0);
                }
                break;

            default:
                break;
            }
        default:
            break;
        }
    }
    exit(0);
}