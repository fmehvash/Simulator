/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "apex_cpu.h"
#include "apex_macros.h"
#include <stdint.h>


/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
void Initialize(APEX_CPU *cpu) {
    cpu->pc = 4000;
    // Initialize other components of the CPU (registers, flags, etc.)
    printf("Simulator initialized. PC set to 4000.\n");
}




static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}



static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_LDR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            
            break;
        }
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }
        

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

         case OPCODE_CML:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1 , stage->imm);
            break;
        }
        case OPCODE_CMP:
                {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1 , stage->rs2);
            break;
        }


        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2, stage->rs3);
            break;
        }



        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BN:
        case OPCODE_BNP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }


    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

// Check dependency and stall condition in decode stage
int check_dependency_in_decode_stage(APEX_CPU *cpu) {
    int stall_detected = FALSE;
    if(cpu->decode.opcode==OPCODE_NOP){
        // cpu->stall=FALSE;
        stall_detected = FALSE;
        return 0;
    }
    

    // Check dependency in execute stage
    if (cpu->execute.has_insn && (cpu->execute.opcode==OPCODE_LDR||cpu->execute.opcode==OPCODE_LOAD )&&
        (cpu->execute.rd == cpu->decode.rs1 && cpu->decode.rs1 != -1||
         cpu->execute.rd == cpu->decode.rs2 && cpu->decode.rs2 != -1 ||
         cpu->execute.rd == cpu->decode.rs3 && cpu->decode.rs3 != -1)) {
        // printf("Stalling due to dependency of decode PC(%d) on execute stage at PC(%d).\n", cpu->decode.pc, cpu->execute.pc);
        stall_detected = TRUE;
    }

    // Check dependency in memory1 stage
    if (!stall_detected && cpu->memory1.has_insn && (cpu->memory1.opcode==OPCODE_LDR||cpu->memory1.opcode==OPCODE_LOAD )&&
        (cpu->memory1.rd == cpu->decode.rs1 && cpu->decode.rs1 != -1||
         cpu->memory1.rd == cpu->decode.rs2 && cpu->decode.rs2 != -1 ||
         cpu->memory1.rd == cpu->decode.rs3 && cpu->decode.rs3 != -1)) {
        // printf("Stalling due to dependency of decode PC(%d) on memory1 stage at PC(%d).\n", cpu->decode.pc, cpu->memory1.pc);
        stall_detected = TRUE;
    }

    // Check dependency in memory stage
    if (!stall_detected && cpu->memory.has_insn && (cpu->memory.opcode==OPCODE_LDR||cpu->memory.opcode==OPCODE_LOAD )&&
        (cpu->memory.rd == cpu->decode.rs1 && cpu->decode.rs1 != -1||
         cpu->memory.rd == cpu->decode.rs2 && cpu->decode.rs2 != -1 ||
         cpu->memory.rd == cpu->decode.rs3 && cpu->decode.rs3 != -1)) {
        // printf("Stalling due to dependency of decode PC(%d) on memory stage at PC(%d).\n", cpu->decode.pc, cpu->memory.pc);
        stall_detected = TRUE;
    }

    

    // Handle stall in decode and fetch stages
    if (stall_detected) {
        cpu->stall = TRUE; 
        
        
         cpu->fetch_from_next_cycle = TRUE;            // Set the stall flag
        // cpu->fetch.has_insn = FALSE;    // Prevent fetch stage from fetching new instructions
        return TRUE;                    // Indicate stall condition detected
    } else {
        cpu->stall = FALSE;             // Clear the stall flag if no dependency
        // cpu->fetch.has_insn = TRUE;     // Allow fetch stage to continue
        return FALSE;
    }
}

// Resolve stall if dependencies are cleared in the writeback stage
void resolve_stall_if_dependency_cleared(APEX_CPU *cpu) {
    if (cpu->stall && cpu->memory.has_insn) {
        // Check if writeback has completed and registers are ready
        
            if (cpu->memory.rd == cpu->decode.rs1 && cpu->decode.rs1 != -1) {
                cpu->rs1_ready = TRUE;
            }
            if (cpu->memory.rd == cpu->decode.rs2 && cpu->decode.rs2 != -1) {
                cpu->rs2_ready = TRUE;
            }
           
        

        // Resolve stall for STR if all registers are ready
     
            if (cpu->rs1_ready || cpu->rs2_ready) {
                // printf("Data hazard resolved for STR; clearing stall.\n");
                cpu->stall = FALSE;
                // cpu->fetch.has_insn = TRUE;  // Re-enable fetching of new instructions
            }
        

        }
    }



static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->halt_pending) 
        {
        
            cpu->fetch.has_insn = FALSE;

            if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("Fetch", &cpu->fetch);
            }
            
            return;
        }

    if (cpu->fetch.has_insn)
    {
        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        cpu->fetch.pc = cpu->pc;
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3= current_ins->rs3;
        cpu->fetch.imm  = current_ins->imm;

        if (cpu->fetch_from_next_cycle == TRUE)
        {
            if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }
            // cpu->fetch.has_insn = TRUE;
            cpu->fetch_from_next_cycle = FALSE;
            return;

        }


        /* Copy data from fetch latch to decode latch*/
    


            cpu->pc += 4;
            cpu->decode = cpu->fetch;
         
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }
    }
}

/*
 * Decode Stage of APEX Pipeline

 *
 * Note: You are free to edit this function according to your implementation
 */
/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->branch_pending == TRUE) {
            cpu->decode.has_insn = FALSE;

        }

// resolve any existing stall from a previous cycle
        if(cpu->decode.opcode!=OPCODE_NOP){
                cpu->stall=FALSE;
                resolve_stall_if_dependency_cleared(cpu);
            }
        // Then, check for new dependencies in the current decode instruction
        if (check_dependency_in_decode_stage(cpu)) {
            // printf("Decode stage is stalled due to a dependency.\n");
            if (ENABLE_DEBUG_MESSAGES)
                {
                    print_stage_content("Decode/RF", &cpu->decode);
                }
                cpu->fetch_from_next_cycle=TRUE;
            return;  // Exit early if a stall is detected
        }

    
    if (cpu->stall == FALSE && cpu->decode.has_insn) {
    {
        // printf("Before decoding: R1= %d, R2 = %d, R3= %d, R4 =%d , R5= %d \n", cpu->regs[1],cpu->regs[2],cpu->regs[3],cpu->regs[4], cpu->regs[5]);
        /* Read operands from register file based on the instruction type */
         if (cpu->decode.opcode == OPCODE_HALT) {
            // printf("HALT instruction encountered. Pipeline will stop fetching new instructions.\n");
            
            cpu->halt_pending = TRUE;  // Set HALT flag
        }

        // Replace instruction in decode stage with NOP if HALT is active
        
        switch (cpu->decode.opcode)
        {
            

            case OPCODE_ADD:
            case OPCODE_MUL:
            case OPCODE_SUB:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_STORE:
            case OPCODE_XOR:
            case OPCODE_LDR:
            case OPCODE_CMP:
            {
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                break;
            }
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:

            {
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                
                break;
            }
            
        
            
   

            case OPCODE_STR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
                break;
            }
            

            case OPCODE_MOVC:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_NOP:

            {
                /*  doesn't have register operands */
                break;
            }
            case OPCODE_JALR:
            {
             
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

            
                break;
            }
            case OPCODE_JUMP:
            {
             
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

            
                break;
            }

            case OPCODE_CML:
            {
                
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                break;
            }

        }

    

        /* Copy data from decode latch to execute latch*/
        cpu->execute = cpu->decode;
        cpu->decode.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}
}


static int forwarding(APEX_CPU *cpu, int reg_id) {
    // Check for forwarding from the Memory stage for LOAD and LDR
    if (cpu->memory1.has_insn && cpu->memory1.rd == reg_id && reg_id != -1) {
        if (cpu->memory1.opcode == OPCODE_LOAD || cpu->memory1.opcode == OPCODE_LDR) {
            printf("Forwarding LOAD/LDR from memory1, value: %d\n", cpu->data_memory[cpu->memory1.memory_address]);
            return cpu->data_memory[cpu->memory1.memory_address];  // Forward value from memory address
        } else {
            printf("Forwarding from memory1, value: %d\n", cpu->memory1.result_buffer);
            return cpu->memory1.result_buffer;  // Forward result buffer for other instructions
        }
    }
    
    if (cpu->memory.has_insn && cpu->memory.rd == reg_id && reg_id != -1) {
        if (cpu->memory.opcode == OPCODE_LOAD || cpu->memory.opcode == OPCODE_LDR) {
            printf("Forwarding LOAD/LDR from memory, value: %d\n", cpu->data_memory[cpu->memory.memory_address]);
            return cpu->data_memory[cpu->memory.memory_address];  // Forward value from memory address
        } else {
            printf("Forwarding from memory, value: %d\n", cpu->memory.result_buffer);
            return cpu->memory.result_buffer;  // Forward result buffer for other instructions
        }
    }

    // Check for forwarding from the Writeback stage
    if (cpu->writeback.has_insn && cpu->writeback.rd == reg_id && reg_id != -1) {
        return cpu->writeback.result_buffer;  // Forward from writeback stage
    }

    // If no forwarding is needed, return the value from the register file
    return cpu->regs[reg_id];
}



static void APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        // Apply forwarding for rs1 and rs2 before executing the instruction
        cpu->execute.rs1_value = forwarding(cpu, cpu->execute.rs1);
        cpu->execute.rs2_value = forwarding(cpu, cpu->execute.rs2);
        cpu->execute.rs3_value = forwarding(cpu, cpu->execute.rs3);

        /* Execute logic based on instruction type */


        // }
switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;
                }
                break;
            }
            case OPCODE_SUB:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;
                }
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;
                }
                
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;
                }
                break;
            }
            case OPCODE_MUL:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;
                }
                break;
            }
            case OPCODE_CML:
            {
                if(cpu->execute.rs1_value == cpu->execute.imm) {
                    cpu->cc.z = 1;
                    printf("Z FLAG is TRUE\n");
                    
                }
                else{
                    cpu->cc.z=0;   

                }
                if(cpu->execute.rs1_value < cpu->execute.imm) {
                    cpu->cc.n = 1;
                    printf("N FLAG is TRUE\n");
                }
                else{
                    cpu->cc.n=0;   
                }
                if(cpu->execute.rs1_value > cpu->execute.imm) {
                    cpu->cc.p = 1;
                    printf("P FLAG is TRUE\n");
                }
                else{
                    cpu->cc.p=0;   
                }
                break;
            }
            case OPCODE_CMP:
            {
                if(cpu->execute.rs1_value == cpu->execute.rs2_value) {
                    cpu->cc.z = 1;
                    printf("Z FLAG is TRUE\n");
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;   
                }

                if(cpu->execute.rs1_value < cpu->execute.rs2_value) {
                    cpu->cc.n = 1;
                    printf("N FLAG is TRUE\n");
                }
                else{
                    cpu->cc.n=0;   
                }
                if(cpu->execute.rs1_value > cpu->execute.rs2_value) {
                    cpu->cc.p = 1;
                    printf("P FLAG is TRUE\n");
                }
                else{
                    cpu->cc.p=0;   
                }
                cpu->cmp_completed = TRUE;
                break;
            }
            case OPCODE_AND:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value & cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;   
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;   
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;   
                }
            }
            case OPCODE_OR:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;   
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;   
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;   
                }
                break;
            }
            case OPCODE_XOR:
            {
                cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;   
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;   
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;   
                }
                break;
            }
            case OPCODE_LOAD:
            {

                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }
            case OPCODE_LDR:
            {
                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
                break;
            }
            case OPCODE_STORE:
            {
                cpu->execute.memory_value = cpu->execute.rs1_value;
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                break;
            }
            case OPCODE_STR:
            {
                cpu->execute.memory_value = cpu->execute.rs1_value;
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.rs3_value;
                break;
            }
             case OPCODE_MOVC:
            {
                cpu->execute.result_buffer = cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->cc.z=1;
                }
                else{
                    cpu->cc.z=0;   
                }
                if (cpu->execute.result_buffer > 0)
                {
                    cpu->cc.p=1;
                }
                else{
                    cpu->cc.p=0;   
                }
                if (cpu->execute.result_buffer< 0)
                {
                    cpu->cc.n=1;
                }
                else{
                    cpu->cc.n=0;   
                }
                break;
            }
            case OPCODE_BZ:
            {
                if (cpu->cc.z == 1) {
                    // Calculate the branch target
                    cpu->branch_target = cpu->execute.pc + cpu->execute.imm;
                    cpu->branch_pending = TRUE; // Mark the branch as pending

                    printf("BZ: Branch target calculated. PC: %d -> New PC: %d\n", cpu->execute.pc, cpu->branch_target);

                    // Stop fetching new instructions since the branch will be taken soon
                    cpu->fetch_from_next_cycle= TRUE;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;
                } else {
                    // No branch is taken if the zero flag is FALSE
                    printf("BZ: No branch taken because zero flag is FALSE.\n");
                }

                
                break;
            }

            

        case OPCODE_BNZ:
            {
                if (cpu->cc.z == 0) {
                    // Calculate the branch target
                    cpu->branch_target = cpu->execute.pc + cpu->execute.imm;
                   
                    cpu->branch_pending = TRUE; // Mark the branch as pending

                    printf("BNZ: Branch target calculated. PC: %d -> New PC: %d\n", cpu->execute.pc, cpu->branch_target);

                    // Stop fetching new instructions since the branch will be taken soon
                    cpu->fetch_from_next_cycle= TRUE;;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;
                } else {
                    // No branch is taken if the zero flag is FALSE
                    printf("BNZ: No branch taken because zero flag is TRUE.\n");
                }

                // Mark the Execute stage as completed for BZ
                // cpu->execute.has_insn = FALSE;
                break;
            }
            case OPCODE_BP:
            {
                if (cpu->cc.p == 1) {
                    // Calculate the branch target
                    cpu->branch_target = cpu->execute.pc + cpu->execute.imm;
                    cpu->branch_pending = TRUE; // Mark the branch as pending

                    printf("BP: Branch target calculated. PC: %d -> New PC: %d\n", cpu->execute.pc, cpu->branch_target);

                    // Stop fetching new instructions since the branch will be taken soon
                    cpu->fetch_from_next_cycle= TRUE; 

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;
                } else {
                    // No branch is taken if the zero flag is FALSE
                    printf("BP: No branch taken because positve flag is FALSE.\n");
                }

                // Mark the Execute stage as completed for BZ
                // cpu->execute.has_insn = FALSE;
                break;
            }
            case OPCODE_BN:
            {
                if (cpu->cc.n == 1) {
                    // Calculate the branch target
                    cpu->branch_target = cpu->execute.pc + cpu->execute.imm;
                    cpu->branch_pending = TRUE; // Mark the branch as pending

                    printf("BN: Branch target calculated. PC: %d -> New PC: %d\n", cpu->execute.pc, cpu->branch_target);

                    // Stop fetching new instructions since the branch will be taken soon
                    cpu->fetch_from_next_cycle= TRUE;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;
                } else {
                    // No branch is taken if the zero flag is FALSE
                    printf("BN: No branch taken because negative flag is FALSE.\n");
                }

                // Mark the Execute stage as completed for BZ
                // cpu->execute.has_insn = FALSE;
                break;
            }
            case OPCODE_BNP:
            {
                if (cpu->cc.n == 1|| cpu->cc.z==1) {
                    // Calculate the branch target
                    cpu->branch_target = cpu->execute.pc + cpu->execute.imm;
                    cpu->branch_pending = TRUE; // Mark the branch as pending

                    printf("BNP: Branch target calculated. PC: %d -> New PC: %d\n", cpu->execute.pc, cpu->branch_target);

                    // Stop fetching new instructions since the branch will be taken soon
                    cpu->fetch_from_next_cycle= TRUE;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;
                } else {
                    // No branch is taken if the zero flag is FALSE
                    printf("BNP: No branch taken because positive is set.\n");
                }

                // Mark the Execute stage as completed for BZ
                // cpu->execute.has_insn = FALSE;
                break;
            }


           
            case OPCODE_JALR: // Define this appropriately
            {

                // Save return address in rd
                cpu->execute.result_buffer = cpu->execute.pc + 4; // Store address of next instruction
            

                cpu->branch_target = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->branch_pending = TRUE;

            //    cpu->fetch_from_next_cycle= TRUE;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;

                break;
            }
            
                case OPCODE_JUMP: // Define this appropriately
            {

              
                cpu->branch_target = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->branch_pending = TRUE;

   
                    // /cpu->fetch_from_next_cycle= TRUE;

                    // Flush the Decode stage to avoid executing wrong path instructions
                    cpu->decode.has_insn = FALSE;

                break;
                
            }
            
            case OPCODE_NOP:{
                break;
            }

        }



        /* Copy data from execute latch to memory latch */
        cpu->memory1 = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory1(APEX_CPU *cpu)
{
    
    
    
        if (!cpu->memory1.has_insn)
            {
                return;
            }

    

else{
    
     if (cpu->branch_pending == TRUE) {
            
            // All previous instructions have completed, so we can safely branch now
            cpu->pc = cpu->branch_target;
            cpu->branch_pending = FALSE;  // Branch has been taken
            printf(" Branch / jump taken. New PC: %d\n", cpu->pc);
            cpu->decode.has_insn = FALSE;
            cpu->execute.has_insn = FALSE;
     }

            // * Convert the jump instruction to an NOP */
           

        if (cpu->memory1.opcode == OPCODE_BZ |cpu->memory1.opcode == OPCODE_BN || cpu->memory1.opcode == OPCODE_BP || 
            cpu->memory1.opcode == OPCODE_BNZ || cpu->memory1.opcode == OPCODE_BNP||cpu->memory1.opcode == OPCODE_JUMP)
        {
            cpu->memory1.opcode = OPCODE_NOP;         // Replace opcode with NOP
            strcpy(cpu->memory1.opcode_str, "NOP");  // Update the opcode string
            cpu->memory1.rd = -1;                    // Clear destination register
            cpu->memory1.rs1 = -1;                   // Clear source registers
            cpu->memory1.rs2 = -1;
            cpu->memory1.rs3 = -1;
            cpu->memory1.imm = 0;                    // Clear immediate value
        } 
            
            

        

        if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("Memory1", &cpu->memory1);
    }

        cpu->memory = cpu->memory1;
    cpu->memory1.has_insn = FALSE;
}
}



static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUB:
            case OPCODE_SUBL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_NOP:
            case OPCODE_JALR:
            case OPCODE_JUMP:


            {
                /* No work for ADD */
                break;
            }
            


            case OPCODE_LOAD:
            {
                /* Read from data memory */
               // printf("MEMORY ADRESS: %d \n",  cpu->memory.memory_address);
                if (cpu->memory.memory_address > 0 && cpu->memory.memory_address < DATA_MEMORY_SIZE){
                cpu->memory.result_buffer= cpu->data_memory[cpu->memory.memory_address];
               //printf("EXTRACTED VALUE FROM MEMORY ADRESS: %d \n",  cpu->memory.result_buffer);
                break;
            }
            }

            case OPCODE_STORE:       {
                /* Read from data memory */
               // printf("MEMORY ADRESS: OF STORE(rs2+#5) %d , MEM VALUE OF STORE:(rs1) %d \n",  cpu->memory.memory_address, cpu->memory.memory_value);
                
                 cpu->data_memory[cpu->memory.memory_address]= cpu->memory.memory_value;

                // printf("final check: %d \n", cpu->data_memory[cpu->memory.memory_address]);
              
    
                break;
            }
            case OPCODE_LDR:
            {
                /* Read from data memory */
               // printf("MEMORY ADRESS: LDR %d \n",  cpu->memory.memory_address);
                if (cpu->memory.memory_address > 0 && cpu->memory.memory_address < DATA_MEMORY_SIZE){
                cpu->memory.result_buffer= cpu->data_memory[cpu->memory.memory_address];
               //printf("EXTRACTED VALUE FROM MEMORY ADRESS LDR: %d \n",  cpu->memory.result_buffer);
                break;
            }
            }

                case OPCODE_STR:       {
                /* Read from data memory */
               // printf("MEMORY ADRESS: OF STORE(rs2+rs3) %d , MEM VALUE OF STORE:(rs1) %d \n",  cpu->memory.memory_address, cpu->memory.memory_value);
                
                 cpu->data_memory[cpu->memory.memory_address]= cpu->memory.memory_value;

    
                break;
            }

        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUB:
            case OPCODE_SUBL:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->writeback.write_complete = TRUE;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->writeback.write_complete = TRUE;
                //printf("Final result in register after loading: %d \n", cpu->regs[cpu->writeback.rd]);
                break;
            }
            case OPCODE_LDR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->writeback.write_complete = TRUE;
               // printf("Final result in register after loading: %d ", cpu->regs[cpu->writeback.rd]);
                break;
            }

            case OPCODE_STORE:
            
            {
                
               // printf("I HAVE DONE MY WORK IN MEMSTAGE! THANK YOU\n");
                break;
            }
            case OPCODE_STR:
            case OPCODE_NOP:

            {
                
              //  printf("I HAVE DONE MY WORK IN MEMSTAGE! THANK YOU\n");
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->writeback.write_complete = TRUE;
                break;
            }
             case OPCODE_JALR:
            {
                // Write the return address to the destination register (if not already done in Execute)
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer; // result_buffer holds the return address

                // Mark the write as complete
                cpu->writeback.write_complete = TRUE;

                // If debugging is enabled, print the action
                if (ENABLE_DEBUG_MESSAGES)
                {
                    printf("Writeback: JALR completed. Return address %d written to register R%d\n",
                        cpu->writeback.result_buffer, cpu->writeback.rd);
                }
                break;
            }
            case OPCODE_JUMP: 
            {
                
                break;
            }

        }
                // If branch is pending, check if all prior instructions have completed
      


            cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

          if (cpu->fetch.has_insn == FALSE&&cpu->writeback.opcode==OPCODE_HALT) {
            return 1;
            
        }
     
        // if (cpu->writeback.opcode == OPCODE_HALT) {
        //     // HALT instruction has reached writeback, stop the simulator
        //     // printf("HALT instruction reached writeback. Stopping simulation.\n");
        //     cpu->halt_pending=FALSE;
        // }
        // if (cpu->memory.opcode==OPCODE_NOP && cpu->writeback.opcode == OPCODE_HALT) {
        //     // HALT instruction has reached writeback, stop the simulator
        //     // printf("HALT instruction reached writeback. Stopping simulation.\n");
        //     cpu->halt_pending=FALSE;
        // }
        // else{
        //     return;
        // }


        // if (cpu->halt_pending==FALSE && cpu->writeback.opcode == OPCODE_NOP) {
        //     // Last NOP after HALT has completed
        //     printf("NOP instruction (converted after HALT) reached writeback. Stopping simulation.\n");
        //     return TRUE;
        // }
        // if (cpu->writeback.opcode == OPCODE_HALT) {
        //     // HALT instruction has reached writeback, stop the simulator
        //     printf("HALT instruction reached writeback. Stopping simulation.\n");
        //     cpu->halt_pending=FALSE;
        //     return TRUE;
        // }

        // Normal writeback logic here...

        // Clear the writeback latch
        // cpu->writeback.has_insn = FALSE;
    }
    return 0;
}

 
void SetMem(APEX_CPU *cpu, const char *filename) {
    FILE *file = fopen(filename, "r"); // Open the data file
    if (file == NULL) {
        perror("Error opening file"); // Print error if file can't be opened
        return;
    }


    int address = 0;
    int data;

    while (fscanf(file, "%d,", &data) != EOF && address < DATA_MEMORY_SIZE) {
        cpu->data_memory[address] = data; // Set data in the CPU's data memory
        address++;
    }
    
    fclose(file); // Close the file
    printf("Memory initialized from file.\n");
    printf("Data Memory Contents:\n");
    for (int i = 0; i < 10; i++) {
        printf("Address %d: %d\n", i, cpu->data_memory[i]);
    }

}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    //cpu->pc = 4000;
    Initialize(cpu);
    cpu->cmp_completed = FALSE;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    /*cpu->data_memory[248] = 10; */// explicitly filling value in data mem
    cpu->single_step = ENABLE_SINGLE_STEP;
    cpu->stall = 0; 
    cpu->cc.z = 0;
    cpu->cc.n = 0;
    cpu->cc.p = 0;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }
    
    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_simulate(APEX_CPU *cpu, int num_cycles) {
    int cycle = 0;

    while (cycle < num_cycles) {
        printf("implementing simulate\n");
        if (ENABLE_DEBUG_MESSAGES) {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu)) {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d, instructions completed = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_memory1(cpu);
        
        APEX_execute(cpu);
        
        APEX_decode(cpu);
        APEX_fetch(cpu);
        print_reg_file(cpu);

        /* Increment both cycle count and cpu->clock */
        cycle++;
        cpu->clock++;

        /* Check if reached the specified number of cycles */
        if (cycle >= num_cycles) {
            printf("APEX_CPU: Simulation stopped after %d cycles, instructions completed = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }
    }
}
// Function to display the current state of the APEX CPU
void APEX_cpu_display(APEX_CPU *cpu) {
    printf("=== APEX CPU State ===\n");
    printf("Program Counter (PC): %d\n", cpu->pc);
    printf("Instructions Completed: %d\n", cpu->insn_completed);
    
    // Display Registers
    printf("\nRegisters:\n");
    for (int i = 0; i < REG_FILE_SIZE; i++) {
        printf("R%d: %d\n", i, cpu->regs[i]);
    }

    // Display Zero Flag
    printf("\nZero Flag: %s\n", cpu->zero_flag ? "TRUE" : "FALSE");

    // Display Data Memory Contents (First 10 locations)
    printf("\nData Memory Contents (First 10 Locations):\n");
    for (int i = 0; i < 50; i++) {
        printf("Data Memory[%d]: %d\n", i, cpu->data_memory[i]);
    }


    printf("======================\n");
}

void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    int num_cycles = 0;
    printf("Do you want to set memory from a file? (y/n): ");
    scanf(" %c", &user_prompt_val); // Note the space before %c to consume newline

    if (user_prompt_val == 'y' || user_prompt_val == 'Y') {
        char filename[256]; // Adjust size as needed
        printf("Enter the filename: ");
        scanf("%s", filename); // Read filename from user
        SetMem(cpu, filename); // Call SetMem with the user-provided filename
    }
    printf("Do you want to simulate? (y/n): ");
    scanf(" %c", &user_prompt_val);
    if (user_prompt_val == 'y' || user_prompt_val == 'Y') {

    printf("Enter the number of cycles to simulate (or enter 0 to run indefinitely): ");
    scanf("%d", &num_cycles);
    }

    if (num_cycles > 0) {
        /* Run simulation for specified number of cycles */
        APEX_cpu_simulate(cpu, num_cycles);
    } 
    else
     {
    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_memory1(cpu);
    
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);
        print_reg_file(cpu);
        
        
        
        // Prompt the user if they want to display the CPU state




        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }


        cpu->clock++;
        
        
        }
    
    printf("Do you want to display the CPU state? (y/n): ");
    scanf(" %c", &user_prompt_val);  // Note the space before %c to consume newline

    if (user_prompt_val == 'y' || user_prompt_val == 'Y') {
        APEX_cpu_display(cpu);  // Call the display function if user wants
            }
        
    }
}


/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu){
    free(cpu->code_memory);
    free(cpu);
}




