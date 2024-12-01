/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    // Test the get_code_memory_index_from_pc function


    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc != 2) { // Expecting two arguments
        fprintf(stderr, "APEX_Help: Usage %s <input.asm> \n", argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    

    return 0;
    
}