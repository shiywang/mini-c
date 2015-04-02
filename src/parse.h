#ifndef PARSE_H
#define PARSE_H
#include "global.h"
#include <string.h>
/*******************MiniC Grammar***********************************
  00.<Define>            ->  <Declaration> {;<Declaration>};<Program>
  01.<Program>           ->  main()<Statement_Block>
  02.<Statement_Block>   ->  '{'<Statement_Sequence>'}'
  03.                      ->  <Statement_Sequence>                                             
  04.<Statement_Sequence>->  <Statement> {;<Statement>} ;
  05.<Statement>         -><EQ> ( VAR or ARRAY )
  06.                      | <IF> 
  07.                      | <WHILE>                    
  08.<EQ>                ->  ID = <Expression>
  09.<IF>                ->  if(<Condition>) <Statement_Block>
  10.                      |   if(<Condition>) <Statement_Block> else <Statement_Bock>
  11.<WHILE>             ->  while(<Condition>) <Statement_Block>
  12.<Condition>         ->  <Expression> != <Expression>
  13.                      |   <Expression> == <Expression>
  14.                      |   <Expression> >= <Expression>
  15.                      |   <Expression> <= <Expression>
  16.                      |   <Expression> == <Expression>
  17.                      |   <Expression> >  <Expression>
  18.                      |   <Expression> <  <Expression>                 
  19.<Expression>        ->  <Term>  {+<Term>}
  20.                      |   <Term>  {-<Term>}                    
  21.<Term>              ->  <Factor> {*<Factor>}
  22.                      |   <Factor> {/<Factor>}                 
  23.<Factor>            ->  ID
  24.                      |   NUM
  25.                      |   ARRAY
  26.                      |   (<Expression>)                                   
 *************************************************************************/
QUAD * pQuad;
Node uWord;
int trueExit,falseExit,nextQ,tempVarId;
extern void PrintQuad();
extern void Parse();

#endif

