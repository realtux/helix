%{
#include <stdio.h>
%}

%union {
    int int_val;
    double double_val;
    char *string_val;
}

%token PLUS MINUS ASTERISK FSLASH
%token LPAREN RPAREN LCURLY RCURLY LBRACKET RBRACKET
%token EQUALS COMPAREAPPROX COMPARESTRICT
%token SEMICOLON QUOTE OUT

%token <string_val> WORD
%token <int_val> NUMBER

%type <string_val> string;
%type <int_val> expression;

%start tree

%%

tree: lines;

lines: lines line | line;

line:
    OUT string SEMICOLON {
        printf("%s", $2);
    }
    |
    OUT expression SEMICOLON {
        printf("%i", $2);
    };

string:
    QUOTE WORD QUOTE {
        $$ = $2;
    };

expression:
    NUMBER
    |
    expression PLUS NUMBER {
        $$ = $1 + $3;
    };

%%
