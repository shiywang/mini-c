#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "code.h"
#include "global.h"

#define IiAddR_SIZE 1024
#define DiAddR_SIZE 1024
#define NR_REGS 8
#define PC_REG 7
#define top_m 1024

#define NO_TYPE 0
extern QUAD * pQuad;
extern int nextQ;
extern listNode* FindTable( const char *name );


instruc_t imem[IiAddR_SIZE];
int iPos = 1;
int jumpList[1000] = {-1};
int used[1000];
static int front = 0, rear = 0;

/* These are instructions *******************/
static void Add( );      
static void Sub( );     	
static void Mul( );
static void Div( );	

static void Ld( int x, int a, int num, int type );// memory load to regs 
static void St( int a, int num, int x, int type );// regs store on memory
static void Lda( );			        // load address to regs
static void Ldc( int x,  char * num );// load const values to regs

static void Jmp( int num );
static void Jlt( int num );
static void Jle( int num );
static void Jgt( int num );
static void Jge( int num );
static void Jeq( int num );
static void Jne( int num );

static void Read( int r );
static void Write( int r );

static void JumpConvertToJump();
static void PrintObjCode();

static int FindTempIndex( char * name )
{
	char *p = &name[1];
	return atoi(p);
}

static int FindArrayIndex( char * name )
{	
	//name = "a[i]"
	//name = "a[3]"
	//name = "a[T1]"
	//name = "a[a[3]]" 
	int addr = 0;
	int i, len;
	char * p;
	listNode *a;
	len = strlen(name);
	for( i = 0; i < len; i++ )
	{
		if( name[i] == '[' )
		{
			p = &name[i+1];
		}
		if( name[i] == ']' )
		{
			name[i] = '\0';
			break;
		}
	}
	a = FindTable( p );
	if( a )
	{
		if( a->syn == SYN_VAR )
			return (addr+a->address);
		else if( a->syn == SYN_ARRAY )
			return addr + FindArrayIndex( p );
	}
	else
	{
		if( p[0] == 'T'  )
		{
			return top_m-FindArrayIndex(p);
		}
		else
			return atoi(p);
	}		
}
static int FindType( char * name )
{
	int i;
	char * p;
	int len = strlen(name);
	for( i = 0; i < len; i++ )
	{
		if( name[i] == '[' )
		{
			p = &name[i+1];
		}
		if( name[i] == ']' )
		{
			name[i] = '\0';
			break;
		}
	}
	if( p[0] == 'T' )
		return SYN_VAR;
	if( p[0] >= '0' && p[0] <= '9' )
		return SYN_NUM;
	else
		return SYN_VAR;
}
static int IsNumber( char x )
{
	if( x >= '0' && x <= '9' )
		return true;
	return false; 
}


void GenCode()
{
	int i, k, x;
	int jumpNum = 0;
	const int r = 0;
	const int s = 1;
	const int t = 2;
	listNode *a, *b, *c;
	memset( used, 0, sizeof(used) );
	JumpConvertToJump( );
	front = 0;
	for( i = 1; i <= nextQ; i++ )
	{
		for( k = 0; k < rear; k++ )
			if( jumpList[k] == i && jumpList[front] != -1 )
			{
				if( !used[k] )
				{
					used[k] = 1;
					jumpList[k] = iPos;
				}
			}
		if( !strcmp( pQuad[i].op, "read" ) )
		{
			a = FindTable( pQuad[i].result );
			if( a->syn == SYN_VAR )
			{
				Read( r );
				St( a->address, 0, r, 0 );
			}
			else if( a->syn == SYN_ARRAY )
			{
				Read( r );	
				St(a->address,FindArrayIndex(pQuad[i].result),r,FindType(pQuad[i].result));		
			}
			else
			{
				fprintf( stderr, "read wrong\n" );
			}
		}
		else if( !strcmp( pQuad[i].op, "write") )
		{
			a = FindTable( pQuad[i].result );
			if( a->syn == SYN_VAR )
			{
				Ld( r, a->address, 0, 0 );
				Write( r );
			}
			else if( a->syn == SYN_ARRAY )
			{
				Ld(r,a->address,FindArrayIndex(pQuad[i].result),FindType( pQuad[i].result ) );
				Write( r );
			}
			else
			{
				fprintf( stderr, "write wrong\n" );
			}
		}
		else if( !strcmp( pQuad[i].op, "==") )
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// == a      a      jump
			// == a[]    a      jump
			// == a     a[]     jump
			// == a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, 0 );
					Ld( t, b->address, 0, 0 );
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType(pQuad[i].argv1));
					Ld( t, b->address, 0, 0 );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, 0 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2)  );
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  ==  a   10   jump
			//  == a[]  10   jump
			//  == a[]	T1   jump
			//  == a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber( pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, 0 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY &&  pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address,FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex( pQuad[i].argv2), 0, 0 );
					Sub();
				}
				else if( a->syn == SYN_VAR &&  pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, 0 );
					Ld( t, top_m-FindArrayIndex( pQuad[i].argv2), 0, 0 );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  ==  10   a   jump
			//  ==  10  a[]   jump
			//  ==  T1	a[]   jump
			//  ==  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber( pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber( pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address,FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// == 10 T1  jump
			// == T1 10  jump
			// == T1 T1  jump
			// == 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jeq( jumpNum );
		}//match if( !strcmp( pQuad[i].op, "==") )
		else if(!strcmp( pQuad[i].op, ">="))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// >= a      a      jump
			// >= a[]    a      jump
			// >= a     a[]     jump
			// >= a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1 ), FindType(pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0 , NO_TYPE);
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  >=  a   10   jump
			//  >= a[]  10   jump
			//  >= a[]	T1   jump
			//  >= a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address,FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address,FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0 , NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  >=  10   a   jump
			//  >=  10  a[]   jump
			//  >=  T1	a[]   jump
			//  >=  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// >= 10 T1  jump
			// >= T1 10  jump
			// >= T1 T1  jump
			// >= 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jge( jumpNum );
		}// match else if(!strcmp( pQuad[i].op, ">="))
		else if(!strcmp( pQuad[i].op, "<="))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// <= a      a      jump
			// <= a[]    a      jump
			// <= a     a[]     jump
			// <= a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0 , NO_TYPE);
					Ld( t, b->address, 0 , NO_TYPE);
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1 ),FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  <=  a   10   jump
			//  <= a[]  10   jump
			//  <= a[]	T1   jump
			//  <= a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  <=  10   a   jump
			//  <=  10  a[]   jump
			//  <=  T1	a[]   jump
			//  <=  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(a->name), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// <= 10 T1  jump
			// <= T1 10  jump
			// <= T1 T1  jump
			// <= 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0]== 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jle( jumpNum );
		}// match else if(!strcmp( pQuad[i].op, ">="))
		else if(!strcmp( pQuad[i].op, "!="))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// != a      a      jump
			// != a[]    a      jump
			// != a     a[]     jump
			// != a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType( pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  !=  a   10   jump
			//  != a[]  10   jump
			//  != a[]	T1   jump
			//  != a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0]== 'T' )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  !=  10   a   jump
			//  !=  10  a[]   jump
			//  !=  T1	a[]   jump
			//  !=  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// != 10 T1  jump
			// != T1 10  jump
			// != T1 T1  jump
			// != 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jne( jumpNum );
		}// match else if(!strcmp( pQuad[i].op, "!="))
		else if(!strcmp( pQuad[i].op, ">"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// > a      a      jump
			// > a[]    a      jump
			// > a     a[]     jump
			// > a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2));
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  >  a   10   jump
			//  > a[]  10   jump
			//  > a[]	T1   jump
			//  > a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  >  10   a   jump
			//  >  10  a[]   jump
			//  >  T1	a[]   jump
			//  >  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// > 10 T1  jump
			// > T1 10  jump
			// > T1 T1  jump
			// > 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0]== 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0 , NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0] =='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jgt( jumpNum );
		}// match else if(!strcmp( pQuad[i].op, ">"))
		else if(!strcmp( pQuad[i].op, "<"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );
			// < a      a      jump
			// < a[]    a      jump
			// < a     a[]     jump
			// < a[]   a[]     jump
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0 , NO_TYPE );
					Ld( t, b->address, 0 , NO_TYPE );
					Sub( );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  <  a   10   jump
			//  < a[]  10   jump
			//  < a[]	T1   jump
			//  < a    T1   jump
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  <  10   a   jump
			//  <  10  a[]   jump
			//  <  T1	a[]   jump
			//  <  T1   a   jump
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// < 10 T1  jump
			// < T1 10  jump
			// < T1 T1  jump
			// < 10 10  jump
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0 , NO_TYPE);
					Sub();
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0] =='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
				}
				else if( IsNumber(a->name[0]) && IsNumber(b->name[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
			Jlt( jumpNum );
		}// match else if(!strcmp( pQuad[i].op, "<"))
		
		else if(!strcmp( pQuad[i].op, "="))
		{
			a = FindTable( pQuad[i].argv1 );
			c = FindTable( pQuad[i].result );
			if( c && a )
			{
				//    a = a;
				if( a->syn == SYN_VAR && c->syn == SYN_VAR )
				{
					Ld( r, a->address, 0, NO_TYPE );  //reg[r] = dMem[a];
					St( c->address, 0, r, NO_TYPE );		// dMem[b] = reg[r]
				}
				//   a = a[];
				else if( c->syn == SYN_VAR && a->syn == SYN_ARRAY )
				{
					Ld( r, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1));
					St( c->address, 0, r, NO_TYPE );
				}
				else if( c->syn == SYN_ARRAY && a->syn == SYN_VAR )
				{
					Ld( r, a->address, 0, NO_TYPE );
					St( c->address, FindArrayIndex(pQuad[i].result), r, FindType(pQuad[i].result));
				}
				else if( c->syn == SYN_ARRAY && a->syn == SYN_ARRAY )
				{
					Ld( r, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					St( c->address, FindArrayIndex(pQuad[i].result), r, FindType(pQuad[i].result) );
				}
				else
				{
					fprintf( stderr, "totally wrong\n " );
				}
			}
			else if( c && !a )
			{
				if( pQuad[i].argv1[0] == 'T' && c->syn == SYN_VAR )
				{
					Ld( r, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					St( c->address, 0, r, NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0])  && c->syn == SYN_VAR )
				{
					Ldc( r, pQuad[i].argv1 );
					St( c->address, 0, r, NO_TYPE );
				}
				else if(pQuad[i].argv1[0] == 'T' && c->syn == SYN_ARRAY )
				{
					Ld( r, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					St( c->address, FindArrayIndex(pQuad[i].result), r, FindType(pQuad[i].result) );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && c->syn == SYN_ARRAY )
				{
					Ldc( r, pQuad[i].argv1 );
					St( c->address, FindArrayIndex(pQuad[i].result), r,FindType(pQuad[i].result) );
				}
				else
				{
					fprintf( stderr, "totally wrong\n " );
				}
			}
			else
			{
				fprintf( stderr, "tollly wrong" );
			}
		}
	
		else if(!strcmp( pQuad[i].op, "+"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );	
			// + a      a      temp
			// + a[]    a      temp
			// + a     a[]     temp
			// + a[]   a[]     temp
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Add( );
					St( r, top_m-FindArrayIndex(pQuad[i].result), 0,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1));
					Ld( t, b->address, 0 , NO_TYPE );
					Add();
					St( r, top_m-FindArrayIndex(pQuad[i].result), 0, NO_TYPE );
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2)  );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  +  a   10   temp
			//  + a[]  10   temp
			//  + a[]  T1   temp
			//  + a    T1   temp
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Add();
					St( r, top_m-FindArrayIndex(pQuad[i].result), 0, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0 , NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0 , r , NO_TYPE);
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  +  10   a   temp
			//  +  10  a[]  temp
			//  +  T1	a[] temp
			//  +  T1   a   temp
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0, NO_TYPE );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0 , r, NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2)  );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2)  );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0 ,NO_TYPE);
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// + 10 T1  temp
			// + T1 10  temp
			// + T1 T1  temp
			// + 10 10  temp
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r ,NO_TYPE);
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Add();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
		}
		else if(!strcmp( pQuad[i].op, "-"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );	
			// - a      a      temp
			// - a[]    a      temp
			// - a     a[]     temp
			// - a[]   a[]     temp
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Sub( );
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r , NO_TYPE);
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType( pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType( pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2), FindType( pQuad[i].argv2) );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  -  a   10   temp
			//  - a[]  10   temp
			//  - a[]  T1   temp
			//  - a    T1   temp
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType( pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r ,NO_TYPE);
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  -  10   a   temp
			//  -  10  a[]  temp
			//  -  T1	a[] temp
			//  -  T1   a   temp
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0 ,NO_TYPE);
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2), FindType( pQuad[i].argv2) );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ld( t, b->address, 0 ,NO_TYPE);
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// - 10 T1  temp
			// - T1 10  temp
			// - T1 T1  temp
			// - 10 10  temp
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE);
					Ldc( t, pQuad[i].argv2 );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Sub();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
		}
		else if(!strcmp( pQuad[i].op, "*"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );	
			// * a      a      temp
			// * a[]    a      temp
			// * a     a[]     temp
			// * a[]   a[]     temp
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0,NO_TYPE );
					Ld( t, b->address, 0,NO_TYPE );
					Mul( );
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1), FindType(pQuad[i].argv1));
					Ld( t, b->address, 0,NO_TYPE );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r ,NO_TYPE);
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  *  a   10   temp
			//  * a[]  10   temp
			//  * a[]  T1   temp
			//  * a    T1   temp
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0,NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1)  );
					Ldc( t, pQuad[i].argv2 );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r ,NO_TYPE);
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0, NO_TYPE );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0,NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  *  10   a   temp
			//  *  10  a[]  temp
			//  *  T1	a[] temp
			//  *  T1   a   temp
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0 ,NO_TYPE);
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2),FindType( pQuad[i].argv2) );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(a->name), 0, NO_TYPE  );
					Ld( t, b->address, FindArrayIndex( pQuad[i].argv2 ),FindType( pQuad[i].argv2 ) );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(a->name), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// * 10 T1  temp
			// * T1 10  temp
			// * T1 T1  temp
			// * 10 10  temp
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(a->name), 0,NO_TYPE );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Mul();
					St( r, top_m-FindArrayIndex(pQuad[i].result), 0,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Mul();
					St( r, top_m-FindArrayIndex(pQuad[i].result), 0,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Mul();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
		}
		else if(!strcmp( pQuad[i].op, "/"))
		{
			a = FindTable( pQuad[i].argv1 );
			b = FindTable( pQuad[i].argv2 );	
			// / a      a      temp
			// / a[]    a      temp
			// / a     a[]     temp
			// / a[]   a[]     temp
			if( a && b )	 
			{
				if( a->syn == SYN_VAR && b->syn == SYN_VAR )
				{
					Ld( s, a->address, 0 ,NO_TYPE);
					Ld( t, b->address, 0 ,NO_TYPE);
					Div( );
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_VAR )
				{
					Ld( s, a->address, FindArrayIndex( pQuad[i].argv1),FindType( pQuad[i].argv1));
					Ld( t, b->address, 0, NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_VAR && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && b->syn == SYN_ARRAY )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1), FindType(pQuad[i].argv1) );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2), FindType(pQuad[i].argv2) );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 117 line wrong" );
			}
			//  /  a   10   temp
			//  / a[]  10   temp
			//  / a[]  T1   temp
			//  / a    T1   temp
			else if( a && !b )
			{
				if( a->syn == SYN_VAR && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, 0, NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ldc( t, pQuad[i].argv2 );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( a->syn == SYN_ARRAY && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, FindArrayIndex(pQuad[i].argv1),FindType(pQuad[i].argv1) );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0 , r,NO_TYPE );
				}
				else if( a->syn == SYN_VAR && pQuad[i].argv2[0] == 'T' )
				{
					Ld( s, a->address, 0,NO_TYPE );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0 ,r,NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 1 line wrong" );
			}
			//  /  10   a   temp
			//  /  10  a[]  temp
			//  /  T1	a[] temp
			//  /  T1   a   temp
			else if( !a && b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_VAR )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, 0,NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && b->syn == SYN_ARRAY )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && b->syn == SYN_ARRAY )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ld( t, b->address, FindArrayIndex(pQuad[i].argv2),FindType(pQuad[i].argv2) );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T'&&  b->syn == SYN_VAR  )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0, NO_TYPE );
					Ld( t, b->address, 0, NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r, NO_TYPE );
				}
				else
					fprintf( stderr, "code.c 2 line wrong" );
			}
			// / 10 T1  temp
			// / T1 10  temp
			// / T1 T1  temp
			// / 10 10  temp
			else if( !a && !b )
			{
				if( IsNumber(pQuad[i].argv1[0]) && pQuad[i].argv2[0] == 'T' )
				{
					Ldc( s, pQuad[i].argv1 );
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] == 'T' && IsNumber(pQuad[i].argv2[0]) )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE );
					Ldc( t, pQuad[i].argv2 );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( pQuad[i].argv1[0] =='T' && pQuad[i].argv2[0]=='T' )
				{
					Ld( s, top_m-FindArrayIndex(pQuad[i].argv1), 0,NO_TYPE);
					Ld( t, top_m-FindArrayIndex(pQuad[i].argv2), 0,NO_TYPE );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
				else if( IsNumber(pQuad[i].argv1[0]) && IsNumber(pQuad[i].argv2[0]) )
				{
					Ldc( s, pQuad[i].argv1 );
					Ldc( t, pQuad[i].argv2 );
					Div();
					St( top_m-FindArrayIndex(pQuad[i].result), 0, r,NO_TYPE );
				}
			}
			else
			{
				fprintf( stderr, "totally wrong\n " );
			}
		}
		else if(!strcmp( pQuad[i].op, "jmp"))
		{
			Jmp( jumpNum );
		}
		else
		{
		//	fprintf( STD_OUT, "op wrong!\n" );
		//	exit(0);
		}
	}
	imem[iPos].iop = opHALT;
	iPos++;
	PrintObjCode();
}

void Read( int r )
{
	imem[iPos].iop = opIN;
	imem[iPos].reg = r;
	iPos++;
}

void Write( int r )
{
	imem[iPos].iop = opOUT;
	imem[iPos].reg = r;
	iPos++;
}

void Add( )
{
	imem[iPos].iop = opADD;
	iPos++;
}
void Sub( )
{
	imem[iPos].iop = opSUB;
	iPos++;
}
void Mul( )
{
	imem[iPos].iop = opMUL;
	iPos++;
}
void Div( )
{
	imem[iPos].iop = opDIV;
	iPos++;
}
void Ld( int x, int a, int num, int type )
{
	imem[iPos].iop = opLD;
	imem[iPos].reg = x;
	imem[iPos].dmem = a;
	imem[iPos].num = num;
	imem[iPos].type = type;
	iPos++;
}
void St( int a, int num, int x, int type )
{
	imem[iPos].iop = opST;
	imem[iPos].reg = x;
	imem[iPos].dmem = a;
	imem[iPos].num = num;
	imem[iPos].type = type;
	iPos++;
}
void Lda( )
{
	fprintf( fpOut, "no use for now" );
}
void Ldc( int x,  char * num )
{
	int a = atoi( num );	
	imem[iPos].iop = opLDC;
	imem[iPos].reg = x;
	imem[iPos].num = a;
	iPos++;
}
void Jmp( int num )
{
	imem[iPos].iop = opJMP;
	imem[iPos].num = num;
	iPos++;
}
void Jlt( int num )
{
	imem[iPos].iop = opJLT;
	imem[iPos].num = num;
	iPos++; 
}
void Jle( int num )
{
	imem[iPos].iop = opJLE;
	imem[iPos].num = num;
	iPos++;
}
void Jgt( int num )
{
	imem[iPos].iop = opJGT;
	imem[iPos].num = num;
	iPos++;
}
void Jge( int num )
{
	imem[iPos].iop = opJGE;
	imem[iPos].num = num;
	iPos++;
}
void Jeq( int num )
{
	imem[iPos].iop = opJEQ;
	imem[iPos].num = num;
	iPos++;
}
void Jne( int num )
{
	imem[iPos].iop = opJNE;
	imem[iPos].num = num;
	iPos++;
}
static void JumpConvertToJump( )
{
	int i;
	// middle code's jump number 
	//convert to object code's jump number 
	rear = 0;
	for( i = 1; i < nextQ; i++ )
	{
		if( (!strcmp( pQuad[i].op, ">=" )) ||
			(!strcmp( pQuad[i].op, "jmp" )) ||
			(!strcmp( pQuad[i].op, ">" )) ||
			(!strcmp( pQuad[i].op, "<" )) ||
			(!strcmp( pQuad[i].op, "<=" )) ||
			(!strcmp( pQuad[i].op, "==" )) ||
			(!strcmp( pQuad[i].op, "!=" )) )
			jumpList[rear++] = atoi(pQuad[i].result);
	}
}

static void PrintObjCode()
{
	int i;
	front = 0;
	for( i = 1; i < iPos; i++ )
	{
		if( imem[i].iop >= opJMP )
		{
			imem[i].num = jumpList[front++];
			switch( imem[i].iop )
			{
				case opJMP:
					fprintf( fpOut, "JMP %d\n", imem[i].num );
					break;
				case opJLT:
					fprintf( fpOut, "JLT %d\n", imem[i].num );
					break;
				case opJLE:
					fprintf( fpOut, "JLE %d\n", imem[i].num );
					break;
				case opJGT:
					fprintf( fpOut, "JGT %d\n", imem[i].num );
					break;
				case opJGE:
					fprintf( fpOut, "JGE %d\n", imem[i].num );
					break;
				case opJEQ:
					fprintf( fpOut, "JEQ %d\n", imem[i].num );
					break;
				case opJNE:
					fprintf( fpOut, "JNE %d\n", imem[i].num );
					break;
			}
		}
		else
		{
			switch( imem[i].iop )
			{
				case opADD:
					fprintf( fpOut, "ADD\n" );
					break;	 
        		case opSUB:
					fprintf( fpOut, "SUB\n" );
					break; 
        		case opMUL:
        			fprintf( fpOut, "MUL\n" );
        			break;
        		case opDIV:
        			fprintf( fpOut, "DIV\n" );
        			break;
        		case opLD:
        			fprintf( fpOut, "LD reg[%d] = dMem[%d]+%d\n", imem[i].reg, imem[i].dmem, imem[i].num );
					break;   
        		case opST:
					fprintf( fpOut, "ST dMem[%d]+%d = reg[%d]\n", imem[i].dmem, imem[i].num, imem[i].reg );
					break;    
				case opLDC:
					fprintf( fpOut, "LDC reg[%d] = %d\n", imem[i].reg, imem[i].num );
					break;    
        		case opLDA:
					break; 
				case opIN:
					fprintf( fpOut, "IN reg[%d]\n", imem[i].dmem );
					break;
				case opOUT:
					fprintf( fpOut, "OUT %d\n", imem[i].reg );
					break; 
			}
		}
	}
}

