/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-18.
 *
 */


/* TODO
 * - add .str's capacity size to Token
 *   for allocating just token's characters length.
 *   (if capacity is a fewer than needed size, use realloc())
 * - replace 'destroy' with 'destruct'
 * - can use TOKEN_ALLOCA() only if 'USE_TOKEN_ALLOCA' is defined
 */


#define _POSIX_C_SOURCE 1
    #include "common.h"
#undef _POSIX_C_SOURCE

#include "util.h"
#include "token.h"
#include "parser.h"

#include "mylib/stack.h"


#include <stdarg.h>








typedef struct {
    Stack   *cur_stack;    // easier to access
    Stack   cur_stack__;
    FILE    *f_in;
    FILE    *f_out;
    FILE    *f_err;
} Dentaku;



/*
 * calculate top 3 elems of stack.
 * result token is tok_result.
 */
static bool
dentaku_calc_op(Dentaku *dentaku, Token *tok_result, Token *tok_n, Token *tok_op, Token *tok_m)
{
    Digit n, m, result;
    double d_n, d_m;
    bool success;


    /* check each token's type */
    if (! (tok_n->type == TOK_DIGIT
        && tok_m->type == TOK_DIGIT
        && tok_op->type == TOK_OP))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n->str, tok_op->str, tok_m->str);
        return false;
    }


    /* convert */
    if (! atod(tok_n->str, &n, 10)) {
        WARN2("can't convert '%s' to digit", tok_n->str);
        return false;
    }
    d_n = digit2double(&n);

    if (! atod(tok_m->str, &m, 10)) {
        WARN2("can't convert '%s' to digit", tok_m->str);
        return false;
    }
    d_m = (double)m.i + m.d;


    /* calc */
    switch (*tok_op->str) {
    case '+': success = double2digit(d_n + d_m, &result); break;
    case '-': success = double2digit(d_n - d_m, &result); break;
    case '*': success = double2digit(d_n * d_m, &result); break;
    case '/': success = double2digit(d_n / d_m, &result); break;
    default:
        WARN2("unknown op '%s'", tok_op->str);
        return false;
    }
    if (! success) {
        WARN("failed to convert double to digit");
        return false;
    }


    /* push result */
    // XXX tok_result->str's length may not be MAX_TOK_CHAR_BUF.
    if (! dtoa(&result, tok_result->str, MAX_TOK_CHAR_BUF, 10)) {
        WARN("can't convert digit to string");
        return false;
    }
    tok_result->type = TOK_DIGIT;

    return true;
}







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


// get one token.
// if this gets EOF, free top_tok.
char*
dentaku_get_token(Dentaku *dentaku, char *src, Token *top_tok)
{
    Stack *stk = dentaku->cur_stack;
    bool allow_signed = stk->top == NULL || ((Token*)stk->top)->type == TOK_LPAREN;

    if (top_tok->str == NULL) {
        token_alloc(top_tok, MAX_TOK_CHAR_BUF);
    }
    src = get_token(src, top_tok, allow_signed);
    if (src == NULL) {
        token_destroy(top_tok);
    }
    return src;
}


// this is function to
// - copy popped token to tok.
// - print debug message
stack_ret
dentaku_stack_pop(Dentaku *dentaku, Token *tok)
{
    Stack *stk = dentaku->cur_stack;

    if (tok)
        memcpy(tok, stk->top, sizeof(Token));

    d_printf("pop! [%s]", ((Token*)stk->top)->str);
    return stack_pop(stk);
}


stack_ret
dentaku_stack_push(Dentaku *dentaku, Token *tok)
{
    d_printf("push! [%s]", ((Token*)dentaku->cur_stack->top)->str);
    return stack_push(dentaku->cur_stack, tok);
}


void
alloc_tokens_for_calc(Token *tok_n, Token *tok_op, Token *tok_m)
{
    token_init(tok_n);
    token_alloc(tok_n, MAX_TOK_CHAR_BUF);

    token_init(tok_op);
    token_alloc(tok_op, MAX_TOK_CHAR_BUF);

    token_init(tok_m);
    token_alloc(tok_m, MAX_TOK_CHAR_BUF);
}


void
destruct_tokens_for_calc(Token *tok_n, Token *tok_op, Token *tok_m)
{
    token_destroy(tok_n);
    token_destroy(tok_op);
    token_destroy(tok_m);
}


/*
 * TOK_RPAREN or EOF:
 *  - repeat while stack sequence is '<op> <digit>'.
 *    - pop top of TOK_DIGIT
 *    - pop top of TOK_LPAREN
 *    - pop top of TOK_DIGIT
 *    - pop if top is TOK_LPAREN
 *    - if top of stack is NULL
 *      - call dentaku_calc_op()
 *      - return
 *    - or else, just call dentaku_calc_op()
 * TOK_OP:
 *  - push it
 *  - if op is '*' or '/'
 *    - get token
 *    - if it is digit, call dentaku_calc_op()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - just push it
 */
bool
dentaku_eval_src(Dentaku *dentaku, char *src)
{
    Stack *stk = dentaku->cur_stack;
    Token got_tok;
    Token *old_top;
    Token tok_op, tok_n, tok_m;

    // char *end_pos;
    // end_pos = src + strlen(src);

        // if (src >= end_pos)
        //     return false;

    while (1) {
        bool allow_signed = stk->top == NULL || ((Token*)stk->top)->type == TOK_LPAREN;
        old_top = stk->top;

        token_init(&got_tok);
        token_alloc(&got_tok, MAX_TOK_CHAR_BUF);
        src = get_token(src, &got_tok, allow_signed);
        // there are no tokens on stack, and parser gets EOF.
        if (src == NULL) {
            if (stk->top == NULL)
                return false;
            else
                memcpy(&got_tok, stk->top, sizeof got_tok);
        }


        // TODO
        // - separate each case into static functions
        // - wrap get_token()
        // - wrap stack_push(), stack_pop()


        if (src == NULL || got_tok.type == TOK_RPAREN) {    // EOF or ')'
            while (1) {
                memcpy(&tok_m, stk->top, sizeof tok_m);
                stack_pop(stk);

                if (stk->top == NULL) {
                    stack_push(stk, &tok_m);
                    return true;
                }

                memcpy(&tok_op, stk->top, sizeof tok_op);
                stack_pop(stk);

                if (stk->top == NULL) {
                    WARN("reaching EOF where digit is expected");
                    return false;
                }

                memcpy(&tok_n, stk->top, sizeof tok_n);
                stack_pop(stk);


                // got_tok is result.
                if (! dentaku_calc_op(dentaku, &got_tok, &tok_n, &tok_op, &tok_m))
                    return false;

                if (stk->top == NULL) {
                    stack_push(stk, &got_tok);
                    return true;
                }
                else {
                    if (((Token*)stk->top)->type == TOK_LPAREN) {    // pop if top is '('
                        token_destroy(stk->top);
                        stack_pop(stk);
                    }
                    stack_push(stk, &got_tok);
                }
            }
        }
        else if (got_tok.type == TOK_OP) {    // '+', '-', '*', '/'
            d_printf("push! [%s]", got_tok.str);
            stack_push(stk, &got_tok);

            // postpone '+' and '-'.
            if (*got_tok.str == '*' || *got_tok.str == '/') {
                token_init(&got_tok);
                token_alloc(&got_tok, MAX_TOK_CHAR_BUF);
                src = get_token(src, &got_tok, allow_signed);
                if (src == NULL) {
                    WARN("reaching EOF where expression is expected");
                    return false;
                }

                d_printf("push! [%s]", got_tok.str);
                stack_push(stk, &got_tok);

                switch (got_tok.type) {
                case TOK_DIGIT:
                    // calculate if parser gets digit.
                    memcpy(&tok_m, stk->top, sizeof tok_m);
                    stack_pop(stk);

                    if (stk->top == NULL) {
                        stack_push(stk, &tok_m);
                        return true;
                    }

                    memcpy(&tok_op, stk->top, sizeof tok_op);
                    stack_pop(stk);

                    if (stk->top == NULL) {
                        WARN("reaching EOF where digit is expected");
                        return false;
                    }

                    memcpy(&tok_n, stk->top, sizeof tok_n);
                    stack_pop(stk);


                    // got_tok is result.
                    if (! dentaku_calc_op(dentaku, &got_tok, &tok_n, &tok_op, &tok_m))
                        return false;
                    stack_push(stk, &got_tok);

                    break;

                // syntax checking
                case TOK_RPAREN:
                    WARN("reaching ')' where expression is expected");
                    return false;
                case TOK_OP:
                    WARN2("reaching '%c' where expression is expected", *got_tok.str);
                    return false;
                }
            }
        }
        else if (got_tok.type == TOK_DIGIT) {
            // just push it
            d_printf("push! [%s]", got_tok.str);
            stack_push(stk, &got_tok);

            // syntax checking
            if (src == NULL) {
                if (old_top) {
                    WARN("reaching EOF where operator is expected");
                    return false;
                }
                d_printf("only one digit");
                return true;
            }
        }
        else if (got_tok.type == TOK_LPAREN) {
            // syntax checking
            if (stk->top && ((Token*)stk->top)->type != TOK_OP) {
                WARN2("reaching '%s' where operator is expected", ((Token*)stk->top)->str);
            }

            // just push it
            d_printf("push! [%s]", got_tok.str);
            stack_push(stk, &got_tok);
        }
        else {
            if (got_tok.str)
                WARN2("unknown token '%s' found", got_tok.str);
            else
                WARN("unknown token found");
            return false;
        }
    }
}

void
dentaku_clear_stack(Dentaku *dentaku)
{
    Token *top = dentaku->cur_stack->top;

    while (dentaku->cur_stack->top) {
        top = dentaku->cur_stack->top;
        d_printf("pop! [%s]", top->str);

        token_destroy(top);
        stack_pop(dentaku->cur_stack);
    }
}


void
dentaku_show_result(Dentaku *dentaku)
{
    Token *top = dentaku->cur_stack->top;

    if (top == NULL)
        return;

    // show result
    puts(top->str);
}






int main(int argc, char *argv[])
{
    char src[MAX_IN_BUF];
    Dentaku dentaku;
    Dentaku *d = &dentaku;

    dentaku_init(d);
    dentaku_alloc(d, MAX_STACK_SIZE);

    while (dentaku_read_src(d, src, MAX_IN_BUF)) {
        if (dentaku_eval_src(d, src)) {
            dentaku_show_result(d);
            dentaku_clear_stack(d);
        }
        else {
            dentaku_clear_stack(d);
        }
    }

    dentaku_destroy(d);


    return EXIT_SUCCESS;
}
