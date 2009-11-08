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
%token ADD SUB MUL DIV CR LP RP
%type <double_value> expression term primary_expression
 %%
line_list
    : line
    | line_list line
    ;
line
    : expression CR
    {
        Token tok;
        Digit d;

        token_init(&tok);
        token_alloc(&tok, MAX_TOK_CHAR_BUF);

        double2digit($1, &d);
        digit2token(&d, &tok, MAX_TOK_CHAR_BUF, 10);
        stack_push(parser_result_stack, &tok);

        //printf(">>%lf\n", $1);
    }
expression
    : term
    | expression ADD term
    {
        $$ = $1 + $3;
    }
    | expression SUB term
    {
        $$ = $1 - $3;
    }
    ;
term
    : primary_expression
    | term MUL primary_expression
    {
        $$ = $1 * $3;
    }
    | term DIV primary_expression
    {
        $$ = $1 / $3;
    }
    ;
primary_expression
    : DOUBLE_LITERAL
    | LP expression RP
    {
        $$ = $2;
    }
    | ADD primary_expression
    {
        $$ = $2;
    }
    | SUB primary_expression
    {
        $$ = -$2;
    }
    ;
 %%


int
yyerror(char const *str)
{
    extern char *yytext;
    fprintf(stderr, "%s: parser error near %s\n", str, yytext);
    return 0;
}

