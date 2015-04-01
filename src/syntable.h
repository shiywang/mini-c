#ifndef SYNTABLE_H
#define SYNTABLE_H
#include "global.h" 
extern int InsertTable( unsigned char syn, 
						 const char *name, int size, int type ); 
extern listNode* FindTable( const char *name );
extern int WatchTable( unsigned char syn, const char * name, int error );
#endif
