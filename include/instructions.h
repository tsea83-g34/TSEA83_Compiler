#ifndef COM_INSTRUCTIONS_H
#define COM_INSTRUCTIONS_H

// Arithmetic
#define ADD_INSTR       "add"
#define SUB_INSTR       "sub"
#define ADD_IMM_INSTR   "addi"
#define SUB_IMM_INSTR   "subi"
#define NEG_INSTR       "neg"

#define CMP_INSTR       "cmp"
#define CMP_IMM_INSTR   "cmpi"

// Logical

#define XOR_INSTR       "xor"

// Branch

#define BREQ_INSTR      "breq"
#define BRNE_INSTR      "brne"
#define BRLT_INSTR      "brlt"
#define BRGT_INSTR      "brgt"
#define BRLE_INSTR      "brle"
#define BRGE_INSTR      "brge"
#define JMP_INSTR       "rjmp"

// Register

#define MOVLO_INSTR     "movlo"
#define MOVHI_INSTR     "movhi"
#define MOVE_INSTR      "move"

#define LOAD_INSTR      "ld"
#define STORE_INSTR     "str"

#define CALL_INSTR      "call"
#define RETURN_INSTR    "ret"

#define PUSH_INSTR      "push"
#define POP_INSTR       "pop"

#endif