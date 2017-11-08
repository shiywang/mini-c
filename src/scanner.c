#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include "global.h"
#include "scanner.h"

static int IsDigtial( char num );
static int Cmp( const void * a, const void *b );
static void InitList( void );
static void InsertList( int flag, char * temp  );
static int IsKeyword( char * temp );
static int readRow = 0;

#include <string.h>

 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}  

/* itoa:  convert n to characters in s */
 void itoa(int n, char s[], int base)
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}  

static int is_digtial( char num )
{
    if( num >= '0' && num <= '9' )
        return true;
    return false;
}
static int Cmp( const void * a, const void *b )
{
    return strcmp( (char*)a, (char*)b );
};
static void InitList( void )
{
    head = (Node)malloc(sizeof(node));
    head->num = 0;
    head->next = NULL;
    head->error_row = 0;
    nextNode = head;
};
static void InsertList( int flag, char * temp  )
{
    Node newNode = (Node)malloc(sizeof(node));
    newNode->num = nextNode->num+1;
    newNode->error_row = readRow;
    if( flag == SYN_VAR )
    {
        strcpy( newNode->word.value.car, temp );
        newNode->word.syn = SYN_VAR;
    }
    else if( flag == SYN_NUM )
    {
        newNode->word.value.number = atoi(temp);
        newNode->word.syn = SYN_NUM;
    }
    else if( flag > SYN_BEGIN && flag <= SYN_END )
    {
        newNode->word.syn = flag;
        strcpy( newNode->word.value.car, temp );
    }
    else
        fprintf(fpOut,"INSERT TYPE ERROR\n");
    newNode->next = NULL;
    nextNode->next = newNode;
    nextNode = nextNode->next;
};
static int IsKeyword( char * temp )
{
    int i;
    for( i = 0; i < KEY_WORD_NUM; i++ )
        if( !strcmp(temp,keyword[i]) )
            return i+1;
    return SYN_VAR;
};
void PrintScan()
{
    Node t;
    fprintf( fpOut, "  id\t  syn\t\tvalue\n\n" );
    for( t = head->next; t != NULL; t = t->next )
    {
        if( t->word.syn == SYN_NUM )
            fprintf( fpOut, "%d\t\t%d\t\t%d\n", t->num, t->word.syn, t->word.value.number );
        else
            fprintf( fpOut, "%d\t\t%d\t\t%s\n", t->num, t->word.syn, t->word.value.car );
    }
}
void Scanner()
{
    int word_pos, num_value, max_word_len;
    char word[100], word_temp[100], input[MAXLEN];
    char *p;

    InitList();
    while( fgets(input,100,fpIn) )
    {
        readRow++;
        strcpy( word, input );
        p = word;
        while( *p != '\n' )
        {
            word_pos = 0;
            while( *p == ' ' || *p == 9 )
                p++;
            if( *p == '\n' )// bug  test file : while( a < 10 {
                break;
            if( isalpha(*p) || *p == '_' )
            {
                CLR(word_temp);
                max_word_len = 0;
                while( isalpha(*p) || is_digtial(*p) || *p == '_' )
                {
                    word_temp[word_pos++] = *p;
                    p++ , max_word_len++;
                }
                if( max_word_len >= 20 )
                {
                    fprintf( fpOut, "word's max len error");
                    exit(0);
                }
                word_temp[word_pos] = '\0';
                if( *(p) == '[' )
                {
                    InsertList( SYN_ARRAY, word_temp );
                }
                else
                    InsertList( IsKeyword(word_temp), word_temp );
            }
            else if( *p >= '0' && *p <= '9' )
            {
                CLR(word_temp);
                num_value = 0;
                while( (*p>='0' && *p<='9') )
                {
                    num_value = num_value * 10 + *p-'0';
                    p++;
                }
                if( isalpha(*p) )
                {
                    fprintf( fpOut, "word wrong can't not begin with number" );
                    exit(0);
                    return ;
                }
                if( num_value >= 32767 )
                {
                    fprintf( fpOut, "int too big" );
                    exit(0);
                    return ;
                }
                else
                {
                    itoa(num_value,word_temp,10);
                    InsertList( SYN_NUM, word_temp );
                }
            }
            else
            { 
                switch( *p )
                {
                    case '=':
                        if( *(p+1) == '=' )
                        {
                            InsertList( SYN_EQ, "==" );
                            p++;
                        }
                        else
                            InsertList( SYN_ASSIGN, "=" );
                        break;
                    case '+':
                        InsertList( SYN_PLUS, "+" );
                        break;
                    case '-':
                        InsertList( SYN_MINUS, "-" );
                        break;
                    case '*':
                        InsertList( SYN_MUL, "*" );
                        break;
                    case '/':
                        InsertList( SYN_DIV, "/" );
                        break;
                    case '<':
                        if( *(p+1) == '=' )
                        {
                            InsertList( SYN_LE, "<=" );
                            p++;
                        }
                        else
                            InsertList( SYN_LT, "<" );
                        break;
                    case '>':
                        if( *(p+1) == '=' )
                        {
                            InsertList( SYN_ME, ">=" );
                            p++;
                        }
                        else
                            InsertList( SYN_LG, ">" );
                        break;
                    case '!':
                        InsertList( SYN_NE, "!=" );
                        break;
                    case ',':
                        InsertList( SYN_COMMA, "," );
                        break;
                    case ':':
                        InsertList( SYN_COLON, ":" );
                        break;
                    case ';':
                        InsertList( SYN_SEMICOLON, ";" );
                        break;
                    case '(':
                        InsertList( SYN_LPAREN, "(" );
                        break;
                    case ')':
                        InsertList( SYN_RPAREN, ")" );
                        break;
                    case '{':
                        InsertList( SYN_LEFTBRACKET2, "{" );
                        break;
                    case '}':
                        InsertList( SYN_RIGHTBRACKET2, "}" );
                        break;
                    case '[':
                        InsertList( SYN_LEFTBRACKET1, "[" );
                        break;
                    case ']':
                        InsertList( SYN_RIGHTBRACKET1, "]" );
                        break;
                    default:
                        return ;
                        break;
                }
                p++;
            }
        }
        }
    }

