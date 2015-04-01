#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"

#define IADDR_SIZE 1024
#define DADDR_SIZE 1024
#define NR_REGS 8
#define PC_REG 7
extern void VmInit( void );
extern void VmRun( void );
extern iPos;
extern instruc_t imem[IADDR_SIZE];

static void insExec( instruc_t* inp );
static void inExec();
static void outExec();
static void addExec();
static void subExec();
static void mulExec();
static void divExec();
static void ldExec();
static void stExec();
static void ldaExec();
static void ldcExec();
static void jmpExec();
static void jltExec();
static void jleExec();
static void jgtExec();
static void jgeExec();
static void jeqExec();
static void jneExec();


int dmem[DADDR_SIZE];
int regs[NR_REGS];
const int r = 0;
const int s = 1;
const int t = 2;
instruc_t *inp = NULL;


void VmInit( void )
{
	int i;
	for( i = 0; i < NR_REGS; i++ )
		regs[i] = 0;
	for( i = 0; i < DADDR_SIZE; i++ )
		dmem[i] = 0;
	regs[PC_REG] = 1;
	printf( "VM_INIT_FINISHED!!\n\n\n" );	
}


void VmRun( void )
{
    inp = &imem[regs[PC_REG]];
    while( inp->iop != opHALT )
    {
       insExec( inp );
       if( inp->iop < opJMP )
			++(regs[PC_REG]);
       inp = &imem[regs[PC_REG]];
    }
    printf( "\n\n\nVM_RUN_FINISHED!!\n" );
    return ;
}

void insExec( instruc_t* inp )
{
        switch( inp->iop )
        {
                case opIN:
                     inExec();
                     break;
                case opOUT:
                     outExec();
                     break;
                case opADD:
					 addExec();
                	break;
                case opSUB:
					 subExec();
					break;
				case opMUL:
					 mulExec();
					break;
				case opDIV:
					 divExec();
					break;
				case opLD:
					 ldExec();
					break;
				case opST:
					 stExec();
					break;
				case opLDA:
					 ldaExec();
					break;
				case opLDC:
					 ldcExec();
					break;	  
				case opJMP:
					 jmpExec();
					break;
        		case opJLT:
        			 jltExec();
        			break;
        		case opJLE:
        			 jleExec();
        			break;
        		case opJGT:
        			 jgtExec();
        			break;
        		case opJGE:
        			 jgeExec();
        			break;
        		case opJEQ:
        			 jeqExec();
        			break;
        		case opJNE:
					 jneExec();
					break;  
        }
}
static void addExec()
{
	regs[r] = regs[s] + regs[t];
}
static void subExec()
{
	regs[r] = regs[s] - regs[t];
}
static void mulExec()
{
	regs[r] = regs[s] * regs[t];
}
static void divExec()
{
	regs[r] = regs[s] / regs[t];
}
static void ldExec()
{
	if( inp->type == SYN_VAR )
		regs[inp->reg] = dmem[inp->dmem+dmem[inp->num]];
	else
		regs[inp->reg] = dmem[inp->dmem+inp->num];
}
static void stExec()
{
	if( inp->type == SYN_VAR )
		dmem[inp->dmem+dmem[inp->num]] = regs[inp->reg];
	else
		dmem[inp->dmem+inp->num] = regs[inp->reg];
}
static void ldaExec()
{
	
}
static void ldcExec()
{
	regs[inp->reg] = inp->num;
	
}
static void jmpExec()
{
	regs[PC_REG] = inp->num;
}
static void jltExec()
{
	if( regs[r] < 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
}
static void jleExec()
{
	if( regs[r] <= 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
}
static void jgtExec()
{
	if( regs[r] > 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
}
static void jgeExec()
{
	if( regs[r] >= 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
	
}
static void jeqExec()
{
	if( regs[r] == 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
}
static void jneExec()
{
	if( regs[r] != 0 )
		regs[PC_REG] = inp->num;
	else
		++(regs[PC_REG]);
}
static void inExec()
{
	scanf( "%d", &regs[r] );
};
static void outExec()
{
	printf( "%d", regs[r] );
};
