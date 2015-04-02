#ifndef SCANNER_H
#define SCANNER_H
#include "global.h"

#define KEY_WORD_NUM 10
char* keyword[KEY_WORD_NUM] =
{
    "main",
    "int",
    "char",
    "if",
    "else",
    "for",
    "while",
    "void",
    "read",
    "write"
};

Node head = NULL;
Node nextNode = NULL;

extern void PrintScan();
extern void Scanner();

#endif
