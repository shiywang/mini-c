#include "global.h"
#include "parse.h"

extern Node head;
extern int WatchTable( int syn, char * name, int error );

static char * Newtemp( void );
static void Next();
static void Error( char * strError );
static int Match( int syn, char * strError );
static void Gen( char * op, char * argv1, char * argv2, char * result );
static int Merg( int p1, int p2 );
static void Bp( int pos, int value );
static char *Factor( void );
static char *Term( void );
static char *Expression( void );
static void Condition( int *etc, int * efc );
static void Statement( int * nChain, int true_exit, int false_exit );
static void Statement_Sequence( int  nChain, int oneline );
static void Statement_Block( int * nChain );
static void Program( void );
static void Define( void );
static void Declaration( void );


static void Next()
{
    uWord = uWord->next;
}

void PrintQuad( void )
{
    int i;
    for( i = 1; i < nextQ; i++ )
    {
        if( !strcmp( pQuad[i].op, "jmp" ) )
        {
            fprintf( fpOut, "%d:%s  \t%s\n",
                    i, 
                    pQuad[i].op,
                    pQuad[i].result );
        }
        else
        {
            fprintf( fpOut, "%d:%s   \t%s   \t%s   \t%s\n",
                    i, 
                    pQuad[i].op,
                    pQuad[i].argv1,
                    pQuad[i].argv2,
                    pQuad[i].result );
        }
    }
}


static void Error( char * strError )
{
    fprintf( STD_OUT, "The syntac error in %d row which is %s",  uWord->error_row, strError );
    return ;
}


static int Match( int syn, char * strError )
{
    if(  syn != SYN_LEFTBRACKET2 && syn != SYN_RIGHTBRACKET2 )
    {
        if( syn == uWord->word.syn)
        {   
            Next();
            return true;
        }
        else    
        {
            Error( strError );
            return false;
        }
    }
    else
    {
        if( syn == uWord->word.syn)
        {   
            Next();
            return true;
        }
        else    
            return false;
    }
}


static void Gen( char * op, char * argv1, char * argv2, char * result )
{
    strcpy( pQuad[nextQ].op, op );
    strcpy( pQuad[nextQ].argv1, argv1 );
    strcpy( pQuad[nextQ].argv2, argv2 );
    strcpy( pQuad[nextQ].result, result );
    nextQ++;
    return ;
}




static char * Newtemp( void )
{
    char * strTempID = ( char*)malloc(MAXLEN);
    sprintf(strTempID, "T%d", ++tempVarId);
    return strTempID;
}


static int Merg( int p1, int p2 )
{
    int Result, p;
    if( p2 == 0 )
        Result = p1;
    else
    {
        Result = p = p2;
        while( atoi(pQuad[p].result) )
        {
            p = atoi( pQuad[p].result );
            sprintf( pQuad[p].result, "%s", p1 );
        }
    }
    return Result;
}


static void Bp( int pos, int value )
{
    int temp;
    while( pos )
    {
        temp = atoi(pQuad[pos].result);
        sprintf( pQuad[pos].result, "%d", value );
        pos = temp;
    }
    return ;
}


static char *Factor( void )
{
    char * eplace;
    char * strTemp;
    char num[20];
    if( uWord->word.syn == SYN_VAR 
            || uWord->word.syn == SYN_NUM
            || uWord->word.syn == SYN_ARRAY )
    {
        if(!WatchTable(uWord->word.syn, uWord->word.value.car,
                    uWord->error_row ))
        {
            exit( 0 );
        }
        eplace = (char*)malloc(MAXLEN); 
        strTemp = (char*)malloc(MAXLEN); 
        if( uWord->word.syn == SYN_VAR )
        {
            sprintf( strTemp, "%s", uWord->word.value.car);
            Next();
        }
        else if( uWord->word.syn == SYN_ARRAY )
        {
            sprintf( strTemp, "%s", uWord->word.value.car );
            Next(); 
            Match( SYN_LEFTBRACKET1, "[" );
            sprintf( eplace, "%s", "[" );
            strcat( eplace, Expression() );
            strcat( eplace, "]" );
            Match( SYN_RIGHTBRACKET1, "]" );
            strcat( strTemp, eplace );  
        }
        else
        {
            sprintf( strTemp, "%d", uWord->word.value.number);
            Next();
        }
    }
    else
    {
        if( uWord->word.syn == SYN_LPAREN )
        {
            Match( SYN_LPAREN, "(" );
            strTemp = Expression();
            Match( SYN_RPAREN, ")" );
        }
    }
    return strTemp;
}


static char *Term( void )
{
    char opp[2], *eplace1, *eplace2, *eplace;
    eplace = eplace1 = Factor();
    while( uWord->word.syn == SYN_MUL
            || uWord->word.syn == SYN_DIV )
    {
        sprintf( opp, "%s", uWord->word.value.car );
        Next();
        eplace2 = Factor();
        eplace = Newtemp();
        Gen( opp, eplace1, eplace2, eplace );
        eplace1 = eplace;
    }
    return eplace;  
}


static char *Expression( void )
{
    char opp[MAXLEN],  *eplace,  eplace1[MAXLEN], eplace2[MAXLEN];
    eplace = (char*)malloc(MAXLEN);
    strcpy( eplace1, Term() );
    strcpy( eplace, eplace1 );
    while( uWord->word.syn == SYN_PLUS 
            || uWord->word.syn == SYN_MINUS )
    {
        sprintf( opp, "%s", uWord->word.value.car );
        Next();
        strcpy( eplace2, Term() );
        strcpy( eplace, Newtemp() );
        Gen( opp, eplace1, eplace2, eplace );
        strcpy( eplace1, eplace);
    }
    return eplace;
}


static void Condition( int *true_exit, int * false_exit )
{
    char opp[3], *eplace1, * eplace2;
    char strTemp[20];
    eplace1 = Expression();
    if( uWord->word.syn <= SYN_NE && uWord->word.syn >= SYN_LG )
    {
        switch( uWord->word.syn )
        {
            case SYN_LT:
            case SYN_LG:
                sprintf( opp, "%s", uWord->word.value.car );
                break;
            default:
                sprintf( opp, "%s", uWord->word.value.car );
                break;
        }
        Next();
        eplace2 = Expression();
        *true_exit = nextQ;
        *false_exit = nextQ+1;
        Gen( opp, eplace1, eplace2, "0" );
        Gen( "jmp", "", "", "0" );
    }
    else
        Error( "wrong");
}


static void Statement( int * nChain, int true_exit, int false_exit )
{
    char strTemp[MAXLEN], eplace[MAXLEN], num[MAXLEN];
    int nChainTemp, nWQUAD;
    switch( uWord->word.syn )
    {
        case SYN_VAR:
            strcpy( strTemp, uWord->word.value.car );
            if(!WatchTable(uWord->word.syn, strTemp, 
                        uWord->error_row ) )
            {
                exit( 0 );
            }
            Next();
            Match( SYN_ASSIGN, "=" );
            strcpy( eplace, Expression() );
            Match( SYN_SEMICOLON, ";" );
            Gen( "=", eplace, "", strTemp );
            *nChain = 0;
            break;
        case SYN_ARRAY:
            strcpy( strTemp, uWord->word.value.car );
            if(!WatchTable(uWord->word.syn, strTemp, 
                        uWord->error_row ) )
            {
                exit( 0 );
            }
            Next();
            Match( SYN_LEFTBRACKET1, "[");

            sprintf( eplace, "%s", "[" );
            strcat( eplace, Expression() );
            strcat( eplace, "]" );
            strcat( strTemp, eplace );
            Match( SYN_RIGHTBRACKET1, "]" );
            Match( SYN_ASSIGN, "=" );
            strcpy( eplace, Expression() );
            Match( SYN_SEMICOLON, ";" );
            Gen( "=", eplace, "", strTemp );
            *nChain = 0;
            break;
        case SYN_READ:
            Next();
            Match( SYN_LPAREN, "(" );
            strcpy( strTemp, uWord->word.value.car );
            if( uWord->word.syn == SYN_VAR )
            {
                if(!WatchTable(uWord->word.syn, strTemp, 
                            uWord->error_row ) )
                {
                    exit( 0 );
                }
                Gen( "read", "", "", strTemp );
                Next();
            }
            else if( uWord->word.syn == SYN_ARRAY )
            {
                strcpy( strTemp, uWord->word.value.car );
                if(!WatchTable(uWord->word.syn, strTemp, 
                            uWord->error_row ) )
                {
                    exit( 0 );
                }
                Next();
                Match( SYN_LEFTBRACKET1, "[");

                sprintf( eplace, "%s", "[" );
                strcat( eplace, Expression() );
                strcat( eplace, "]" );

                Match( SYN_RIGHTBRACKET1, "]");
                strcat( strTemp, eplace );
                Gen( "read", "", "", strTemp );
            }
            else
            {
                fprintf( stderr, "read function wrong!\n" );
                exit( 0 );
            }
            Match( SYN_RPAREN, ")" );
            Match( SYN_SEMICOLON, ";" );
            break;
        case SYN_WRITE:
            Next();
            Match( SYN_LPAREN, "(" );
            if( uWord->word.syn == SYN_VAR )
            {
                strcpy( strTemp, uWord->word.value.car );
                if(!WatchTable(uWord->word.syn, strTemp, 
                            uWord->error_row ) )
                {
                    exit( 0 );
                }
                Gen( "write", "", "", strTemp );
                Next(); 
            }
            else if( uWord->word.syn == SYN_ARRAY )
            {
                strcpy( strTemp, uWord->word.value.car );
                if(!WatchTable(uWord->word.syn, strTemp, 
                            uWord->error_row ) )
                {
                    exit( 0 );
                }               
                Next();
                Match( SYN_LEFTBRACKET1, "[");
                sprintf( eplace, "%s", "[" );
                strcat( eplace, Expression() );
                strcat( eplace, "]" );
                Match( SYN_RIGHTBRACKET1, "]");
                strcat( strTemp, eplace );
                Gen( "write", "", "", strTemp );
            }
            else
            {
                fprintf( stderr, "write function wrong!\n" );
                exit( 0 );
            }
            Match( SYN_RPAREN, ")" );
            Match( SYN_SEMICOLON, ";" );
            *nChain = 0;
            break;
        case SYN_IF:
            Match( SYN_IF, "if" );
            Match( SYN_LPAREN, "(" );
            Condition( &true_exit, &false_exit );
            Bp( true_exit, nextQ );
            Match( SYN_RPAREN, ")" );
            Statement_Block( &nChainTemp );
            if( uWord->word.syn == SYN_ELSE )
            {
                nChainTemp = nextQ;
                Match( SYN_ELSE, "else" );
                Gen( "jmp", "", "", "0" );  
                Bp( false_exit, nextQ );
                Statement_Block( &nChainTemp );
                *nChain = nChainTemp;
            }
            else
            {
                *nChain = Merg( nChainTemp, false_exit );
            }
            break;
        case SYN_WHILE:
            Match( SYN_WHILE, "while" );
            nWQUAD = nextQ;
            Match( SYN_LPAREN, "(" );
            Condition( &true_exit, &false_exit );
            Bp( true_exit, nextQ );
            Match( SYN_RPAREN, ")" );
            Statement_Block( &nChainTemp );
            sprintf( strTemp, "%d", nWQUAD );
            Gen( "jmp", "", "", strTemp );
            *nChain = false_exit;
            break; 
    }
    return ;
}

static void Statement_Sequence( int  nChain, int oneline )
{
    Statement(&nChain, 0, 0 );
    if( oneline == false )
    {
        while( uWord->word.syn == SYN_VAR
                || uWord->word.syn == SYN_READ
                || uWord->word.syn == SYN_WRITE
                || uWord->word.syn == SYN_IF
                || uWord->word.syn == SYN_WHILE 
                || uWord->word.syn == SYN_ARRAY )
        {
            if(!WatchTable(uWord->word.syn, uWord->word.value.car,
                        uWord->error_row))
            {
                exit( 0 );
            }
            Bp( nChain, nextQ );
            Statement(&nChain, 0, 0 );
        }

    }
    Bp( nChain, nextQ );
    return ;
}


static void Statement_Block( int * nChain )
{
    int oneline = false;
    if( Match( SYN_LEFTBRACKET2, "{" ) )
    {
        Statement_Sequence( *nChain, oneline );
        Match( SYN_RIGHTBRACKET2, "}" );
    }
    else
    {
        oneline = true;
        Statement_Sequence( *nChain, oneline );
        oneline = false;
    }
}

static void Program( void )
{
    int nChain;
    Match( SYN_MAIN, "main" );
    Match( SYN_LPAREN, "(" );
    Match( SYN_RPAREN, ")" );
    Statement_Block( &nChain );
};

static void Define( void )
{
    do
    {
        Declaration();

    }while( uWord->word.syn != SYN_MAIN );
    Program( );
}

static void Declaration( void )
{
    int syn = uWord->word.syn;
    char name[MAXLEN];
    switch( syn )
    {
        case SYN_INT:
            Next();
            if( uWord->word.syn == SYN_VAR )
            {
                strcpy( name, uWord->word.value.car );
                if( !FindTable(  name ) )
                {
                    InsertTable( uWord->word.syn, name );
                    Next();
                }
                else
                {
                    fprintf( STD_OUT, "word already exist\n" );
                    exit(0);
                }
            }
            else if( uWord->word.syn == SYN_ARRAY )
            {
                strcpy( name, uWord->word.value.car );
                if( !FindTable( name ) )
                {
                    Next();
                    Match( SYN_LEFTBRACKET1, "[" );
                    InsertTable( SYN_ARRAY, name, uWord->word.value.number );
                    Match( SYN_NUM, "error number index" );
                    Match( SYN_RIGHTBRACKET1, "]");
                }
                else
                {
                    fprintf( STD_OUT, "word already exist\n" );
                    exit(0);
                }
            }
            else
            {
                fprintf( STD_OUT, "type wrong\n" );
                exit(0);
            }
            break;
        case SYN_CHAR:
            Next();
            if( uWord->word.syn == SYN_VAR )
            {
                strcpy( name, uWord->word.value.car );
                if( !FindTable(  name ) )
                {
                    InsertTable( uWord->word.syn, name );
                    Next();
                }
                else
                {
                    fprintf( STD_OUT, "word already exist\n" );
                    exit(0);
                }
            }
            else if( uWord->word.syn == SYN_ARRAY )
            {
                strcpy( name, uWord->word.value.car );
                if( !FindTable( name ) )
                {
                    Next();
                    Match( SYN_LEFTBRACKET1, "[" );
                    InsertTable( SYN_ARRAY, name, uWord->word.value.number );
                    Match( SYN_NUM, "error number index" );
                    Match( SYN_RIGHTBRACKET1, "]");
                }
                else
                {
                    fprintf( STD_OUT, "word already exist\n" );
                    exit(0);
                }

            }
            else
            {
                fprintf( STD_OUT, "type wrong\n" );
                exit(0);
            }
            break;          
    }
    Match( SYN_SEMICOLON, ";" );
}

void Parse( )
{
    pQuad = (QUAD*)malloc(1000*sizeof(QUAD) );
    tempVarId = 0;
    trueExit = falseExit = nextQ = 1;
    uWord = head;
    Next( );
    Define( );
}
