%{
    
    
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

";"				                return ';';

"<"                             return '<';

">"                             return '>';

">="                            return '>=';

"<="                            return '<=';

"=="                            return EQ;

"!="                            return NEQ;

"++"                            return INCR;

"--"                            return DECR;

"+="                            return AEQ;

"-="                            return SEQ;giat

"*="                            return MEQ;

"/="                            return DEQ;

"and" | "&&"			        return AND;     

"or" | "||"	                    return OR;

"not" | "!"                     return NOT;

"+"                             return ADD;
 
"-"				                return SUB;
				
"*"				                return MUL;
				 
"/"				                return DIV;

"="   			                return ASSIGN;

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

"##.*"                          { return CMNT; }

"#\*(\*(?!#)|[^*])*\*#"         { return MLTI_CMNT; }

"\".*\"" | "\'.*\'"             { return STR_CONST; }

"-"?{digit}+                    { return INT_CONST; }

"-"?{digit}+"."{digit}+         { return DCML_CONST; }

{letter_}({letter_}|{digit})*   { return ID; }

.

%%

int yywrap(){
    return 1;
}
