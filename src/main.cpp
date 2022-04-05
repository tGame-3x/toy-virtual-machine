#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "JMemory.h"
#include "JRegister.h"
#include "JFlag.h"
#include "../utils/file_helper.h"
#include "../utils/interupt_helper.h"
#include "../utils/option_code.h"

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

uint16_t *memory;
uint16_t *reg;
uint16_t check_key()
{
    // for linux keyboard listener
    // fd_set readfds;
    // FD_ZERO(&readfds);
    // FD_SET(STDIN_FILENO, &readfds);

    // struct timeval timeout;
    // timeout.tv_sec = 0;
    // timeout.tv_usec = 0;
    // return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

int main(int argc, const char *argv[])
{
    printf("lc3 vm start ...\n");
    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    JMemory j_memory;
    JRegister j_register;

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j], j_memory.getMemory()))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    // SIGINT	程序终止(interrupt)信号
    signal(SIGINT, handle_interrupt);
    memory = j_memory.getMemory();
    reg = j_register.getRegister();

    /* since exactly one condition flag should be set at any given time, set the Z flag */
    j_register.setRegister(REGISTER::R_COND, FLAG_OP::FL_ZRO);

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum
    {
        PC_START = 0x3000
    };
    j_register.setRegister(REGISTER::R_PC, PC_START);
    bool running = true;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(reg[REGISTER::R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
        case OPTION::OP_ADD:
        {
            /* destination register (DR) */
            uint16_t r0 = (instr >> 9) & 0x7;
            /* first operand (SR1) */
            uint16_t r1 = (instr >> 6) & 0x7;
            /* whether we are in immediate mode */
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                reg[r0] = reg[r1] + imm5;
            }
            else
            {
                uint16_t r2 = instr & 0x7;
                reg[r0] = reg[r1] + reg[r2];
            }

            update_flags(r0);
        }
        break;
        case OP_AND:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                reg[r0] = reg[r1] & imm5;
            }
            else
            {
                uint16_t r2 = instr & 0x7;
                reg[r0] = reg[r1] & reg[r2];
            }
            update_flags(r0);
        }
        break;
        case OP_NOT:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;

            reg[r0] = ~reg[r1];
            update_flags(r0);
        }
        break;
        case OP_BR:
        {
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            uint16_t cond_flag = (instr >> 9) & 0x7;
            if (cond_flag & reg[R_COND])
            {
                reg[R_PC] += pc_offset;
            }
        }
        break;
        case OP_JMP:
        {
            /* Also handles RET */
            uint16_t r1 = (instr >> 6) & 0x7;
            reg[R_PC] = reg[r1];
        }
        break;
        case OP_JSR:
        {
            uint16_t long_flag = (instr >> 11) & 1;
            reg[R_R7] = reg[R_PC];
            if (long_flag)
            {
                uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                reg[R_PC] += long_pc_offset; /* JSR */
            }
            else
            {
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1]; /* JSRR */
            }
            break;
        }
        break;
        case OP_LD:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            reg[r0] = mem_read(reg[R_PC] + pc_offset);
            update_flags(r0);
        }
        break;
        case OP_LDI:
        {
            /* destination register (DR) */
            uint16_t r0 = (instr >> 9) & 0x7;
            /* PCoffset 9*/
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            /* add pc_offset to the current PC, look at that memory location to get the final address */
            reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
            update_flags(r0);
        }
        break;
        case OP_LDR:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t offset = sign_extend(instr & 0x3F, 6);
            reg[r0] = mem_read(reg[r1] + offset);
            update_flags(r0);
        }
        break;
        case OP_LEA:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            reg[r0] = reg[R_PC] + pc_offset;
            update_flags(r0);
        }
        break;
        case OP_ST:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            mem_write(reg[R_PC] + pc_offset, reg[r0]);
        }
        break;
        case OP_STI:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
        }
        break;
        case OP_STR:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t offset = sign_extend(instr & 0x3F, 6);
            mem_write(reg[r1] + offset, reg[r0]);
        }
        break;
        case OP_TRAP:
            switch (instr & 0xFF)
            {
            case TRAP_GETC:
                /* read a single ASCII char */
                reg[R_R0] = (uint16_t)getchar();
                update_flags(R_R0);
                break;
            case TRAP_OUT:
                putc((char)reg[R_R0], stdout);
                fflush(stdout);
                break;
            case TRAP_PUTS:
            {
                /* one char per word */
                uint16_t *c = memory + reg[R_R0];
                while (*c)
                {
                    putc((char)*c, stdout);
                    ++c;
                }
                fflush(stdout);
            }
            break;
            case TRAP_IN:
            {
                printf("Enter a character: ");
                char c = getchar();
                putc(c, stdout);
                fflush(stdout);
                reg[R_R0] = (uint16_t)c;
                update_flags(R_R0);
            }
            break;
            case TRAP_PUTSP:
            {
                /* one char per byte (two bytes per word)
                   here we need to swap back to
                   big endian format */
                uint16_t *c = memory + reg[R_R0];
                while (*c)
                {
                    char char1 = (*c) & 0xFF;
                    putc(char1, stdout);
                    char char2 = (*c) >> 8;
                    if (char2)
                        putc(char2, stdout);
                    ++c;
                }
                fflush(stdout);
            }
            break;
            case TRAP_HALT:
                puts("HALT");
                fflush(stdout);
                running = 0;
                break;
            }
            break;
        case OP_RES:
        case OP_RTI:
        default:
            abort();
            break;
        }
    }
    // restore_input_buffering();

    // for (int i = 0x3000; i < 0x3000 + 10; i++)
    // {
    //     printf("%x \n", j_memory.getMemory()[i]);
    // }
    printf("lc3 vm close ...\n");
}