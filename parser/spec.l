%{
#include <stdio.h>
#include "spec.y.h"

int line_num = 1;
%}

%%
\n        ++line_num;
"out"     return OUT;
"="       return EQUALS;
"=="      return COMPAREAPPROX;
"==="     return COMPARESTRICT;
"+"       return PLUS;
"-"       return MINUS;
"*"       return ASTERISK;
"/"       return FSLASH;
"("       return LPAREN;
")"       return RPAREN;
"{"       return LCURLY;
"}"       return RCURLY;
"["       return LBRACKET;
"]"       return RBRACKET;
"'"       return QUOTE;
";"       return SEMICOLON;
[A-Za-z]+ {
    yylval.string_val = strdup(yytext);
    return WORD;
}
[0-9]+ {
    yylval.int_val = atoi(yytext);
    return NUMBER;
}
. ;
%%

void yyerror(char *err) {
    printf("parse error: %s\n", err);
}

int yywrap(void) {

}

int main(int argc, char **args) {
    FILE *file = fopen(args[1], "r");

    if (file == NULL) {
        printf("couldn't open file for reading\n");
        return 0;
    }

    yyin = file;
    yyparse();

    fclose(file);

    printf("\n");
    printf("executed %d lines", line_num);
    printf("\n");
}
