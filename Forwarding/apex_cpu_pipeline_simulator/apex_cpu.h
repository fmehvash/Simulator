/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"
#include <stdbool.h>

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int rs3;
    int imm;
    int N;
    int P;
    int Z;
    int cc;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs3;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int rs3_value;
    int result_buffer;
    int memory_address;
    int memory_value;
    int has_insn;
    bool stall;
    bool rs1_ready;
    bool rs2_ready;
    bool write_complete;
    bool cmp_completed;
    bool cml_completed;
    bool zero_flag;
    int halt_pending;
    
    int branch_target;
    bool branch_pending;
} CPU_Stage;

typedef struct {
    int z;  // Zero flag
    int n;  // Negative flag
    int p;  // Positive flag
}ConditionCodes;


/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                  /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    bool stall;
    bool rs1_ready;
    bool rs2_ready;
    bool rs3_ready;

    bool write_complete;
    bool cmp_completed;
    bool cml_completed;
    int halt_pending;
    int branch_target;
    bool branch_pending;
    ConditionCodes cc;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory1;
    CPU_Stage memory;
    CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void SetMem(APEX_CPU *cpu, const char *filename);
void Initialize(APEX_CPU *cpu);
void APEX_cpu_simulate(APEX_CPU *cpu, int num_cycles);
#endif
