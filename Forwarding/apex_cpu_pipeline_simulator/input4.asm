MOVC R3,#4
MOVC R4,#16
MOVC R0,#4044
MOVC R1,#1
MOVC R2,#0
JALR R8,R0,#0
ADDL R3,R3,#4
ADDL R4,R4,#4
SUBL R1,R1,#1
BNZ #-16
HALT 
LOAD R5,R3,#0
LOAD R6,R4,#0
ADD R2,R6,R5
STORE R2,R3,#0
JUMP R8,#0
NOP 
NOP