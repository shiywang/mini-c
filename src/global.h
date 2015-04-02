#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYN_BEGIN          0

#define SYN_MAIN           1
#define SYN_INT            2
#define SYN_CHAR           3
#define SYN_IF             4
#define SYN_ELSE           5
#define SYN_FOR            6
#define SYN_WHILE          7
#define SYN_VOID           8
#define SYN_READ           9
#define SYN_WRITE          10
#define SYN_VAR            11
#define SYN_ARRAY          12
#define SYN_FUN            13

#define SYN_NUM            20
#define SYN_ASSIGN         21
#define SYN_PLUS           22
#define SYN_MINUS          23
#define SYN_MUL            24
#define SYN_DIV            25
#define SYN_LPAREN         26
#define SYN_RPAREN         27
#define SYN_LEFTBRACKET1   28
#define SYN_RIGHTBRACKET1  29
#define SYN_LEFTBRACKET2   30
#define SYN_RIGHTBRACKET2  31
#define SYN_COMMA          32
#define SYN_COLON          33
#define SYN_SEMICOLON      34
#define SYN_LG             35
#define SYN_LT             36
#define SYN_ME             37
#define SYN_LE             38
#define SYN_EQ             39
#define SYN_NE             40

#define SYN_END            100
#define SYN_ERROR          101
#define MAXLEN             102


#define STD_IN stdin
#define STD_OUT stdout
#define STD_ERR stderr
#define true 1
#define false 0
#define CLR(val) memset(val,0,sizeof(val))
#define MAX_SYN 1000


/* Lexical analysis */
typedef union Union_WORD
{
    char car[MAXLEN];
    int number;
}Union_WORD;
typedef struct WORD
{
    int syn;
    Union_WORD value;
}WORD;
typedef struct NODE
{
    WORD word;
    int num;
    int error_row;
    struct NODE * next;
}node, *Node;

/* Parsing && Analysis => middle code */
typedef struct QUAD
{
    char op[MAXLEN];
    char argv1[MAXLEN];
    char argv2[MAXLEN];
    char result[MAXLEN];
}QUAD;

/* middle code => vm code */
typedef enum
{
    opHALT,   //just stopexecution, r, s, t all ignored 
    opIN,     //Read a value into regs[r], s and t ignored 
    opOUT,    //Write regs[r] to the terminal, s and t ignored
    opADD,    //regs[r] = regs[s] + regs[t] 
    opSUB,    //regs[r] = regs[s] - regs[t]
    opMUL,    //regs[r] = regs[s] * regs[t]
    opDIV,    //regs[r] = regs[s] / regs[t]
    opRRLIM,  //meaningless
    opLD,     //reg[x] = dMem[a]
    opST,     //dMem[a] = reg[x]
    opRMLIM,  //meaningless
    opLDA,    // reg[x] = a
    opLDC,    // reg[x] = d
    opRSLIM,
    opJMP,    // no condition jump
    opJLT,    //if regs[r] < 0, regs[PC_REG] = regs[s]+t
    opJLE,    //if regs[r] <= 0,regs[PC_REG] = regs[s]+t
    opJGT,    //if regs[r] > 0, regs[PC_REG] = regs[s]+t
    opJGE,    //if regs[r] >= 0,regs[PC_REG] = regs[s]+t
    opJEQ,    //if regs[r] == 0,regs[PC_REG] = regs[s]+t
    opJNE,    //if regs[r] != 0,regs[PC_REG] = regs[s]+t
    opRALIM
}opCode;
typedef struct
{
    opCode iop;
    int reg;
    int dmem;
    int num;
    int type;
}instruc_t;


/* symtable hash bucket */
typedef struct listnode
{
    unsigned char syn;
    char * name;
    int address;
    int size;
    struct listnode * next;
}listNode;
typedef struct bucket
{
    listNode * slot;
}bucket;

/* file pointer */
FILE * fpIn, * fpOut;
#endif

