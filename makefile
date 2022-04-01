parser: y.tab.c lex.yy.c y.tab.h
	gcc y.tab.c lex.yy.c Utils/symbol_table.c Utils/ast.c semantics.c -o compiler
lex.yy.c: lexer.l
	lex lexer.l
y.tab.c: parser.y
	yacc -v -d parser.y
clean:
	rm -f parser y.tab.c lex.yy.c y.tab.h y.output compiler