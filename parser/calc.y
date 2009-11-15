/* via http://kmaebashi.com/programmer/devlang/yacclex.html */

%{
#include "libdatastruct/stack.h"

#include "common.h"
#include "digit.h"
#include "op.h"
#include "token.h"
#include "util.h"

extern unsigned int stack_push(stack_t*, const void*);
// Push result to this stack.
stack_t *parser_result_stack;


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YYDEBUG 1
%}

%union {
    Digit digit;
}

%token <digit>      DOUBLE_LITERAL
%token DENTAKU_ADD DENTAKU_SUB DENTAKU_MUL DENTAKU_DIV DENTAKU_POW DENTAKU_LP DENTAKU_RP DENTAKU_NL
%type <digit> expression term primary_expression

 %%
line_list
    : line
    | line_list line
    ;
line
    : expression DENTAKU_NL
    {
        Token tok;

        token_init(&tok);
        token_alloc(&tok, MAX_TOK_CHAR_BUF);
        digit2token(&$1, &tok, MAX_TOK_CHAR_BUF, 10);

        stack_push(parser_result_stack, &tok);
    }
expression
    : term
    | expression DENTAKU_ADD term
    {
        op_plus(&$$, &$1, &$3);
    }
    | expression DENTAKU_SUB term
    {
        op_minus(&$$, &$1, &$3);
    }
    ;
term
    : primary_expression
    | term DENTAKU_MUL primary_expression
    {
        op_multiply(&$$, &$1, &$3);
    }
    | term DENTAKU_DIV primary_expression
    {
        op_divide(&$$, &$1, &$3);
    }
    | term DENTAKU_POW primary_expression
    {
        op_power(&$$, &$1, &$3);
    }
    ;
primary_expression
    : DOUBLE_LITERAL
    | DENTAKU_LP expression DENTAKU_RP
    {
        digit_copy(&$$, &$2);
    }
    | DENTAKU_ADD primary_expression
    {
        digit_copy(&$$, &$2);
    }
    | DENTAKU_SUB primary_expression
    {
        op_unary_minus(&$$, &$2);
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

