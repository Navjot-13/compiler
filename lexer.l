%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
%}

digit [0-9]
letter [A-Za-z]
letter_ [A-Za-z_]

%%

"begin"                         return BGN;

"{"				                return '{';

"}"				                return '}';

"("				                return '(';

")"				                return ')';

"["				                return '[';

"]"				                return ']';

","				                return ',';

";"				                return SCOL;

"<"                             return '<';

">"                             return '>';

">="                            return GEQ;

"<="                            return LEQ;

"=="                            return EQ;

"!="                            return NEQ;

"++"                            return INCR;

"--"                            return DECR;

"+="                            return AEQ;

"-="                            return SEQ;

"*="                            return MEQ;

"/="                            return DEQ;

"and"|"&&"			        return AND;     

"or"|"||"	                    return OR;

"not"|"!"                     return NOT;

"+"                             return ADD;
 
"-"				                return SUB;
				
"*"				                return MUL;
				 
"/"				                return DIV;

"="   			                return ASSIGN;

"input"                         return INPUT;

"print"                         return PRINT;

"break"                         return BRK;

"true"                          return TRU;

"false"                         return FLS;

"int"                           return INT;

"decimal"                       return DCML;

"bool"                          return BOOL;

"string"                        return STR;

"array"                         return ARR;

"if"                            return IF;

"else"                          return ELSE;

"ifloop"                        return LP;

"return"                        return RETURN;

##.*                            ;

"#\*(\*(?!#)|[^*])*\*#"         ;

\".*\"|\'.*\'                   { strcpy(yylval.str_val, yytext); return STR_CONST; }

{digit}+                        { yylval.int_val = atoi(yytext); return INT_CONST; }

{digit}+"."{digit}+             { yylval.double_val = atof(yytext); return DCML_CONST; }

{letter_}({letter_}|{digit})*   { strcpy(yylval.str_val, yytext); return ID; }

.

%%

int yywrap(){
    return 1;
}
