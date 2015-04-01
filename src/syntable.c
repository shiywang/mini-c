#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syntable.h"
#include "global.h"
#define SHIFT 4
#define HASH_SIZE 211


static bucket hashtable[HASH_SIZE];
int mallocSize = 0;

static int hash( const char* key )
{
    int temp=0;
    int i=0;
    while(key[i]!='\0')
    {
        temp = ((temp<<SHIFT)+key[i])%HASH_SIZE;
        i++;
    }
    return temp;
}

static void InitTable( )
{
	int i;
	for( i = 0; i < HASH_SIZE; i++ )
	{
		hashtable[i].slot = NULL;
	}
}

listNode* FindTable( const char * input )
{
	int i;
	char name[200];
	strcpy( name, input );
	for( i = 0; i < strlen(name); i++ )
		if( name[i] == '[' )
		{
			name[i] = '\0';
			break;
		}
	int key = hash( name );
	if( hashtable[key].slot == NULL )
		return NULL;
	if( hashtable[key].slot != NULL 
			&& hashtable[key].slot->name == NULL )
		fprintf( stderr, "name is null\n" );
	if( !strcmp(hashtable[key].slot->name, name) )
		return hashtable[key].slot;
	else
	{
		listNode * p = hashtable[key].slot->next;
		while( p )
		{
			if( p->name == NULL )
				fprintf( stderr, "name is null\n" );
			if( !strcmp( p->name, name) )
				return p;
			p = p->next;
		}
		return NULL; 
	}
}

int InsertTable( unsigned char syn, const char *name, int size, int type )
{
	int key;
	listNode *p, *q;
	key = hash(name);
	if( name == NULL )
		return false;
	if( hashtable[key].slot == NULL )
	{
		if( syn == SYN_VAR )
		{
			hashtable[key].slot = (listNode*)malloc(sizeof(listNode));
			hashtable[key].slot->syn = syn;
			hashtable[key].slot->address = mallocSize;
			mallocSize++;
			hashtable[key].slot->name = (char*)malloc(sizeof(strlen(name)+1));
			strcpy( hashtable[key].slot->name, name );
			hashtable[key].slot->next = NULL;
			return true;
		}
		else if( syn == SYN_ARRAY )
		{
			hashtable[key].slot = (listNode*)malloc(sizeof(listNode));
			hashtable[key].slot->syn = syn;
			hashtable[key].slot->size = size;
			hashtable[key].slot->address = mallocSize;
			mallocSize += size;
			hashtable[key].slot->name = (char*)malloc(sizeof(strlen(name)+1));
			strcpy( hashtable[key].slot->name, name );
			hashtable[key].slot->next = NULL;
			return true;
		}
		else
			fprintf( stderr, "insert var type don't exist ");
	}
	else
	{
		p = hashtable[key].slot->next;
		q = hashtable[key].slot;
		while( p )
		{
			q = p;
			p = p->next;
		}
		q->next = p;
		p = (listNode*)malloc(sizeof(listNode));
		p->next = NULL;
		if( syn == SYN_VAR )
		{
			p->syn = syn;
			p->address = mallocSize;
			mallocSize++; 
			p->name = (char*)malloc(sizeof(strlen(name)+1));
			strcpy( p->name, name );
			return true;
		}
		else if( syn == SYN_ARRAY )
		{
			p->syn = syn;
			p->size = size;
			p->address = mallocSize;
			mallocSize += size;
			p->name = (char*)malloc(sizeof(strlen(name)+1));
			strcpy( p->name, name );
			return true;
		}
	}
	return false;
}

void PrintTable( void )
{
	int i;
	fprintf( STD_OUT, "\n\n" );
	fprintf( STD_OUT, "SYN\t\t\tVALUE\n" );
	for( i = 0; i < HASH_SIZE; i++ )
	{
		if( hashtable[i].slot != NULL )
		{
			listNode *p = hashtable[i].slot->next; 
			while( p )
			{
				if( p->name == NULL )
					fprintf( stderr, "name is null\n" );
				printf( "syn:%d,\t\tname:%s\n", p->syn, p->name );
				p = p->next;
			}
		}
	}
}

int WatchTable( unsigned char syn, const char * name, int error )
{
	if( syn==SYN_VAR||syn==SYN_ARRAY||syn==SYN_FUN )
	{	
		if( !FindTable(name) )
		{
			fprintf( STD_OUT, "don't exist this var\n" );
			fprintf( STD_OUT, "wrong in %d line", error );
			return false;
		}
		return true;
	}
	return true;
}







