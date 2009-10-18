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
    d_printf("push! [%s]", tok->str);
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
    Token tok_top;
    Token *old_tok_top;
    Token tok_op, tok_n, tok_m;


    while (1) {
        old_tok_top = stk->top;

        token_init(&tok_top);
        token_alloc(&tok_top, MAX_TOK_CHAR_BUF);
        src = dentaku_get_token(dentaku, src, &tok_top);
        if (src == NULL) {
            token_destroy(&tok_top);
            if (stk->top == NULL)
                // there are no tokens on stack, and parser gets EOF.
                return false;
            else
                memcpy(&tok_top, stk->top, sizeof tok_top);
        }
        else {
            dentaku_stack_push(dentaku, &tok_top);
        }


        // TODO
        // - separate each case into static functions


        if (src == NULL || tok_top.type == TOK_RPAREN) {    // EOF or ')'
            if (tok_top.type == TOK_RPAREN) {
                token_destroy(stk->top);
                dentaku_stack_pop(dentaku, NULL);
            }

            // XXX
            // if input is '1 + 2 * (3 + 4) + 5'.
            //
            // when '1 + 2 * (3 + 4)':
            //      expression should be '1 + 2 * 7'
            //      but then it gotta get one more token.
            //      so expression becomes '1 + 2 * 7 + 5'.
            //      and after that,
            //
            //      '1 + 2 * 12'
            //      '1 + 24'
            //      '25'
            //
            //      but correct answer is '20'.

            while (1) {
                dentaku_stack_pop(dentaku, &tok_m);
                if (stk->top == NULL) {
                    dentaku_stack_push(dentaku, &tok_m);
                    return true;
                }

                dentaku_stack_pop(dentaku, &tok_op);
                if (stk->top == NULL) {
                    WARN("reaching EOF where digit is expected");

                    // other token's will be destructed at dentaku_destroy().
                    token_destroy(&tok_m);
                    token_destroy(&tok_op);

                    return false;
                }

                dentaku_stack_pop(dentaku, &tok_n);


                // tok_top is result.
                token_init(&tok_top);
                token_alloc(&tok_top, MAX_TOK_CHAR_BUF);
                bool success = dentaku_calc_op(dentaku, &tok_top, &tok_n, &tok_op, &tok_m);

                token_destroy(&tok_m);
                token_destroy(&tok_op);
                token_destroy(&tok_n);

                if (! success) {
                    token_destroy(&tok_top);
                    return false;
                }
                else if (stk->top == NULL) {
                    // XXX needs this block?
                    dentaku_stack_push(dentaku, &tok_top);
                    return true;
                }
                else {
                    if (((Token*)stk->top)->type == TOK_LPAREN) {    // pop if top is '('
                        token_destroy(stk->top);
                        dentaku_stack_pop(dentaku, NULL);
                    }
                    dentaku_stack_push(dentaku, &tok_top);
                }
            }
        }
        else if (tok_top.type == TOK_OP) {    // '+', '-', '*', '/'
            // TODO
            // - check also stack_(push|pop)'s return value.

            // postpone '+' and '-'.
            if (tok_top.str[0] == '*' || tok_top.str[0] == '/') {
                token_init(&tok_top);
                token_alloc(&tok_top, MAX_TOK_CHAR_BUF);
                src = dentaku_get_token(dentaku, src, &tok_top);
                if (src == NULL) {
                    token_destroy(&tok_top);
                    WARN("reaching EOF where expression is expected");
                    return false;
                }

                dentaku_stack_push(dentaku, &tok_top);

                switch (tok_top.type) {
                case TOK_DIGIT:
                    // calculate if parser gets digit.
                    dentaku_stack_pop(dentaku, &tok_m);
                    if (stk->top == NULL) {
                        dentaku_stack_push(dentaku, &tok_m);
                        return true;
                    }

                    dentaku_stack_pop(dentaku, &tok_op);
                    if (stk->top == NULL) {
                        WARN("reaching EOF where digit is expected");
                        return false;
                    }

                    dentaku_stack_pop(dentaku, &tok_n);


                    // tok_top is result.
                    if (! dentaku_calc_op(dentaku, &tok_top, &tok_n, &tok_op, &tok_m))
                        return false;
                    dentaku_stack_push(dentaku, &tok_top);

                    break;

                // syntax checking
                case TOK_RPAREN:
                    WARN("reaching ')' where expression is expected");
                    return false;
                case TOK_OP:
                    WARN2("reaching '%c' where expression is expected", *tok_top.str);
                    return false;
                }
            }
        }
        else if (tok_top.type == TOK_DIGIT) {
            // syntax checking
            if (old_tok_top && old_tok_top->type == TOK_DIGIT) {
                WARN2("reaching '%s' where operator is expected", old_tok_top->str);
            }
        }
        else if (tok_top.type == TOK_LPAREN) {
            // nop.
            //
            // TODO
            // because I want calc to assume '(expr)(expr)' as '(expr)*(expr)'.
        }
        else {
            if (tok_top.str)
                WARN2("unknown token '%s' found", tok_top.str);
            else
                WARN("unknown token found");
            return false;
        }
    }
}

void
dentaku_clear_stack(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;

    while (stk->top) {
        token_destroy(stk->top);
        dentaku_stack_pop(dentaku, NULL);
    }
}


void
dentaku_show_result(Dentaku *dentaku)
{
    Token *top = dentaku->cur_stack->top;
    if (top)
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
