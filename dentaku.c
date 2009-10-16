/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-16.
 *
 */


/* TODO
 * - paren
 */



#include "util.h"
#include "token.h"
#include "parser.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "../mylib/stack.h"









typedef struct {
    Stack   *cur_stack;    // easier to access
    Stack   cur_stack__;
    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;
} Dentaku;


void
dentaku_init(Dentaku *dentaku)
{
    d_printf("initializing dentaku...");

    dentaku->cur_stack = &dentaku->cur_stack__;
    dentaku->f_in  = stdin;
    dentaku->f_out = stdout;
    dentaku->f_err = stderr;
}

void
dentaku_alloc(Dentaku *dentaku, size_t stack_size)
{
    d_printf("allocating dentaku...");

    // allocate Token's stack
    stack_ret ret = stack_init(dentaku->cur_stack, stack_size, sizeof(Token));
    if (ret != STACK_SUCCESS) {
        WARN("failed to initialize stack");
        exit(EXIT_FAILURE);
    }
}

void
dentaku_destroy(Dentaku *dentaku)
{
    d_printf("destroying dentaku...");

    if (stack_destruct(dentaku->cur_stack) != STACK_SUCCESS)
        WARN("failed to destruct stack");
}



bool
dentaku_read_src(Dentaku *dentaku, char *src, size_t maxsize)
{
    d_printf("dentaku_read_src()");

    if (fileno(dentaku->f_in) == fileno(stdin)) {
        fputs(PROMPT_STR, dentaku->f_out);
        // read each line
        if (fgets(src, maxsize, dentaku->f_in) == NULL)
            return false;
    }
    else {
    }

    d_printf("read! [%s]", src);
    return true;
}


static void
dentaku_calc_op(Dentaku *dentaku, Token *tok_op, Token *tok_n, Token *tok_m)
{
    Token tok_result;
    Digit n, m, result;
    double d_n, d_m;
    bool success;

    // check each token's type
    if (! (tok_n->type == TOK_DIGIT
        && tok_m->type == TOK_DIGIT
        && tok_op->type == TOK_OP))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n->str, tok_op->str, tok_m->str);
        return;
    }


    if (! atod(tok_n->str, &n, 10)) {
        WARN2("can't convert '%s' to digit", tok_n->str);
        return;
    }
    d_n = (double)n.i + n.d;

    if (! atod(tok_m->str, &m, 10)) {
        WARN2("can't convert '%s' to digit", tok_m->str);
        return;
    }
    d_m = (double)m.i + m.d;


    switch (*tok_op->str) {
    case '+': success = double2digit(d_n + d_m, &result); break;
    case '-': success = double2digit(d_n - d_m, &result); break;
    case '*': success = double2digit(d_n * d_m, &result); break;
    case '/': success = double2digit(d_n / d_m, &result); break;
    default:
        WARN2("unknown op '%s'", tok_op->str);
        return;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        return;
    }


    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dtoa(&result, tok_result.str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        return;
    }
    tok_result.type = TOK_DIGIT;

    stack_push(dentaku->cur_stack, &tok_result);
}


void
dentaku_compile_tokens(Dentaku *dentaku, char *src)
{
    Token cur_tok;
    char *after_pos;
    char *end_pos;
    end_pos = src + strlen(src);

    d_printf("dentaku_compile_tokens()");

    while (*src) {
        token_init(&cur_tok);

        // get one token
        bool allow_signed = dentaku->cur_stack->top == NULL || ((Token*)dentaku->cur_stack->top)->type == TOK_LPAREN;    // XXX
        after_pos = get_token(src, &cur_tok, allow_signed);
        if (after_pos == NULL)
            break;
        if (after_pos >= end_pos)
            break;
        src = after_pos;

        // push one token
        d_printf("push! [%s]", cur_tok.str);
        stack_push(dentaku->cur_stack, &cur_tok);
    }
}

void
dentaku_eval_syntree(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;
    Token tok_n, tok_m, tok_op, tok_result;

    d_printf("dentaku_eval_syntree()");

    if (stk->top == NULL) {
        puts("no value on the stack.");
        return;
    }

    while (true) {
        // tok_m
        memcpy(&tok_m, stk->top, sizeof(Token));
        stack_pop(stk);

        if (stk->top == NULL) {
            stack_push(stk, &tok_m);
            return;
        }

        // tok_op
        memcpy(&tok_op, stk->top, sizeof(Token));
        stack_pop(stk);

        if (stk->top == NULL) {
            WARN3("malformed expression after '%s %s ...'.",
                                    tok_m.str, tok_op.str);
            return;
        }

        // tok_n
        memcpy(&tok_n, stk->top, sizeof(Token));
        stack_pop(stk);


        dentaku_calc_op(dentaku, &tok_op, &tok_n, &tok_m);

        token_destroy(&tok_n);
        token_destroy(&tok_m);
        token_destroy(&tok_op);
    }
}

void
dentaku_show_result(Dentaku *dentaku)
{
    Token *top = dentaku->cur_stack->top;

    d_printf("dentaku_show_result()");

    if (top == NULL)
        return;

    // show result
    puts(top->str);

    // clear current stack
    while (dentaku->cur_stack->top) {
        top = dentaku->cur_stack->top;
        d_printf("pop! [%s]", top->str);

        token_destroy(top);
        stack_pop(dentaku->cur_stack);
    }
}






int main(int argc, char *argv[])
{
    char src[MAX_IN_BUF];
    Dentaku dentaku;
    Dentaku *d = &dentaku;

    dentaku_init(d);
    dentaku_alloc(d, MAX_STACK_SIZE);

    while (dentaku_read_src(d, src, MAX_IN_BUF)) {
        dentaku_compile_tokens(d, src);
        dentaku_eval_syntree(d);
        dentaku_show_result(d);
    }

    dentaku_destroy(d);


    return EXIT_SUCCESS;
}
