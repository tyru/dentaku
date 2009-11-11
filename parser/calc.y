/* via http://kmaebashi.com/programmer/devlang/yacclex.html */

%{

#include "token.h"
#include "util.h"

extern unsigned int stack_push(stack_t*, const void*);
// Push result to this stack.
stack_t *parser_result_stack;


#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 1
%}
%union {
    int          int_value;
    double       double_value;
}
%token <double_value>      DOUBLE_LITERAL
%token YYTOK_ADD YYTOK_SUB YYTOK_MUL YYTOK_DIV YYTOK_LP YYTOK_RP YYTOK_NL
%type <double_value> expression term primary_expression
 %%
line_list
    : line
    | line_list line
    ;
line
    : expression YYTOK_NL
    {
        Token tok;
        Digit d;

        token_init(&tok);
        token_alloc(&tok, MAX_TOK_CHAR_BUF);

        double2digit($1, &d);
        digit2token(&d, &tok, MAX_TOK_CHAR_BUF, 10);
        stack_push(parser_result_stack, &tok);
    }
expression
    : term
    | expression YYTOK_ADD term
    {
        $$ = $1 + $3;
    }
    | expression YYTOK_SUB term
    {
        $$ = $1 - $3;
    }
    ;
term
    : primary_expression
    | term YYTOK_MUL primary_expression
    {
        $$ = $1 * $3;
    }
    | term YYTOK_DIV primary_expression
    {
        $$ = $1 / $3;
    }
    ;
primary_expression
    : DOUBLE_LITERAL
    | YYTOK_LP expression YYTOK_RP
    {
        $$ = $2;
    }
    | YYTOK_ADD primary_expression
    {
        $$ = $2;
    }
    | YYTOK_SUB primary_expression
    {
        $$ = -$2;
    }
    ;
 %%


int
yyerror(char const *str)
{
    extern char *yytext;
    fprintf(stderr, "%s: parser error near '[%c(ascii:%d)]%s'\n",
                    str, *yytext, *yytext, yytext+1);
    return 0;
}

