%{
    
    
%}

digit [0-9]
letter [A-Za-z]
letter_ [A-Za-z_]
keyword ( begin | void | int | decimal | bool | char | string | array | if | else | elseif | ifloop | return | and | or | not )

%%

"begin"                         return BGN;

"{"				                return '{';

"}"				                return '}';

"("				                return '(';

")"				                return ')';

"["				                return '[';

"]"				                return ']';

","				                return ',';

";"				                return ';';

"+"                             return ADD;
 
"-"				                return SUB;
				
"*"				                return MUL;
				 
"/"				                return DIV;

"="   			                return ASSIGN;

"=="                            return EQ;

"!="                            return NEQ;

"true"                          return TRU;

"false"                         return FLS;

"and"			                return AND;     

"or"		                    return OR;

"not"                           return NOT;

"int"                           return INT;

"decimal"                       return DCML;

"double"                        return DBL;

"char"                          return CHAR;

"bool"                          return BOOL;

"string"                        return STR;

keyword                         { return KW;}

\".*\"                          { return STR_CONST; }

\'{letter}\'                    { return CHAR_CONST; }

"-"?{digit}+                    { return INT_CONST; }

"-"?{digit}+"."{digit}+         { return DCML_CONST; }

{letter_}({letter_}|{digit})*   { return ID; }

.

%%

int yywrap(){
    return 1;
}
