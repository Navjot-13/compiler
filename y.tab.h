/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ADD = 258,
    SUB = 259,
    MUL = 260,
    DIV = 261,
    ASSIGN = 262,
    EQ = 263,
    NEQ = 264,
    TRU = 265,
    FLS = 266,
    AND = 267,
    OR = 268,
    NOT = 269,
    INT = 270,
    DCML = 271,
    BOOL = 272,
    STR = 273,
    SCOL = 274,
    ID = 275,
    BGN = 276,
    AEQ = 277,
    MEQ = 278,
    SEQ = 279,
    DEQ = 280,
    INCR = 281,
    DECR = 282,
    GEQ = 283,
    LEQ = 284,
    ARR = 285,
    IF = 286,
    ELSE = 287,
    LP = 288,
    BRK = 289,
    RETURN = 290,
    CMNT = 291,
    MLTI_CMNT = 292,
    INP = 293,
    INT_CONST = 294,
    DCML_CONST = 295,
    STR_CONST = 296
  };
#endif
/* Tokens.  */
#define ADD 258
#define SUB 259
#define MUL 260
#define DIV 261
#define ASSIGN 262
#define EQ 263
#define NEQ 264
#define TRU 265
#define FLS 266
#define AND 267
#define OR 268
#define NOT 269
#define INT 270
#define DCML 271
#define BOOL 272
#define STR 273
#define SCOL 274
#define ID 275
#define BGN 276
#define AEQ 277
#define MEQ 278
#define SEQ 279
#define DEQ 280
#define INCR 281
#define DECR 282
#define GEQ 283
#define LEQ 284
#define ARR 285
#define IF 286
#define ELSE 287
#define LP 288
#define BRK 289
#define RETURN 290
#define CMNT 291
#define MLTI_CMNT 292
#define INP 293
#define INT_CONST 294
#define DCML_CONST 295
#define STR_CONST 296

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 43 "parser.y"

  int int_val;
  double double_val;
  char str_val[300];

#line 145 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
