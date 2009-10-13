/* vim:ts=4:sw=4:sts=0:tw=0:set et: */
/* 
 * dentaku.c - calculator
 *
 * Written By: tyru <tyru.exe@gmail.com>
 * Last Change: 2009-10-13.
 *
 */


/* TODO
 * - paren
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "../mylib/stack.h"




#define NDEBUG 1



#define PROMPT_STR  "=> "
#define MAX_IN_BUF          1024
#define MAX_TOK_CHAR_BUF    32
#define MAX_STACK_SIZE      128


#define STREQ(s1, s2)       (*(s1) == *(s2) && strcmp((s1), (s2)) == 0)
#define ALLOCATED(ptr)      ((ptr) != NULL && errno != ENOMEM)
#define UNUSED(x)           ((void)x)




/*** debug ***/

void
d_printf(const char *fmt, ...)
{
#if NDEBUG
    va_list ap;

    fputs("[debug]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
#else
    UNUSED(fmt);
#endif
}




/*** util ***/

#define WARN(msg)           warn(__LINE__, msg)
#define WARN2(msg, arg1)    warn(__LINE__, msg, arg1)
#define WARN3(msg, arg1, arg2)    warn(__LINE__, msg, arg1, arg2)
#define WARN4(msg, arg1, arg2, arg3)    warn(__LINE__, msg, arg1, arg2, arg3)

void
warn(int line, const char *fmt, ...)
{
    va_list ap;

    fputs("[warning]::", stderr);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
    fflush(stderr);
}


#define DIE(msg)    die(__LINE__, msg)

void
die(int line, const char *msg)
{
    fprintf(stderr, "[error]::[%s] at %d\n", msg, line);
    exit(EXIT_FAILURE);
}


// almost code from 'man 3 strtol'.
// if failed, *failed is not NULL.
double
atod(const char *digit_str, int base, char **failed)
{
    char *end_ptr;
    double val;

    UNUSED(base);

    errno = 0;
    val = strtod(digit_str, &end_ptr);

    if (errno == ERANGE || (errno != 0 && val == 0)) {
        *failed = (char*)digit_str;
        return (double)0;
    }
    if (end_ptr == digit_str) {
        *failed = (char*)digit_str;
        return (double)0;
    }
    if (*end_ptr != '\0') {
        *failed = (char*)digit_str;
        return (double)0;
    }

    return val;
}


// on success, return true.
bool
dtoa(double digit, char *ascii, size_t max_size, int base)
{
    UNUSED(base);

    snprintf(ascii, max_size, "%f", digit);
    return true;
}





/*** token ***/

typedef enum {
    TOK_UNDEF,
    TOK_DIGIT,
    TOK_OP,
    TOK_LPAREN,
    TOK_RPAREN,
} TokenType;


typedef struct {
    char        *str;
    TokenType   type;
} Token;


void
token_init(Token *tok)
{
    tok->str  = NULL;
    tok->type = TOK_UNDEF;
}

void
token_alloc(Token *tok, size_t size)
{
    tok->str = malloc(size);
    if (! ALLOCATED(tok->str)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void
token_destroy(Token *tok)
{
    if (tok->str) {
        free(tok->str);
        tok->str = NULL;
    }
}





/*** parser ***/

// if EOF: return NULL
// else: return the first pointer to the non-space character
static char*
skip_space(char *src)
{
    while (*src)
        if (*src == ' ' || *src == '\t' || *src == '\n')
            src++;
        else
            return src;
    return NULL;
}

// if not digit or EOF: return NULL
static char*
get_digit(char *src, char *buf, size_t maxsize)
{
    int pos;
    bool dot_pos = 0;

    // check if top of src is digit
    for (pos = 0; src[pos]; pos++) {
        if (src[pos] == '.') {
            if (dot_pos) {
                // src has '.' already
                WARN2("parsing error near [%s]", src);
                return NULL;
            }
            dot_pos = pos;
        }
        else if (! isdigit(src[pos])) {
            break;
        }
    }
    if (pos == 0) {
        return NULL;
    }
    if (dot_pos > 0 && (dot_pos == 0 || dot_pos == strlen(src) - 1)) {
        WARN("'.' at the head or tail of src");
        return NULL;
    }

    if (pos > maxsize)
        pos = maxsize;
    // copy
    strncpy(buf, src, pos);
    buf[pos] = '\0';

    return src + pos;
}

// if EOF: return NULL
char*
get_token(char *src, Token *tok)
{
    char *after_pos = NULL;
    char tok_buf[MAX_TOK_CHAR_BUF];

    d_printf("get_token()");

    char *incl_src = skip_space(src);
    if (incl_src == NULL)
        return NULL;
    src = incl_src;


    switch (*src) {
    case '(':
        d_printf("token - (");

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_LPAREN;

        src++;
        break;

    case ')':
        d_printf("token - )");

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_RPAREN;

        src++;
        break;

    case '+':
    case '-':
    case '*':
    case '/':
        d_printf("token - op [%c]", *src);

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_OP;

        src++;
        break;

    default:
        // digit
        if (isdigit(*src)) {
            d_printf("token - digit [%c]", *src);

            after_pos = get_digit(src, tok_buf, MAX_TOK_CHAR_BUF);
            if (after_pos == NULL) {
                DIE("malformed digit");
            }
            src = after_pos;

            tok->type = TOK_DIGIT;

            break;
        }

        // other
        d_printf("[%c] [%s]", *src, src);
        DIE("syntax error");
    }

    // allocate just token's length
    size_t alloc_num = strlen(tok_buf) + 1;
    token_alloc(tok, alloc_num);
    strncpy(tok->str, tok_buf, alloc_num);

    d_printf("got! [%s]", tok->str);

    return src;
}





/*** dentaku ***/

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
    char *failed;
    double n, m, result;

    // check each token's type
    if (! (tok_n->type == TOK_DIGIT
        && tok_m->type == TOK_DIGIT
        && tok_op->type == TOK_OP))
    {
        WARN4("expression '%s %s %s' is invalid",
                tok_n->str, tok_op->str, tok_m->str);
        return;
    }


    failed = NULL;
    n = atod(tok_n->str, 10, &failed);
    if (failed) {
        WARN2("can't convert '%s' to digit", tok_n->str);
        return;
    }

    failed = NULL;
    m = atod(tok_m->str, 10, &failed);
    if (failed) {
        WARN2("can't convert '%s' to digit", tok_m->str);
        return;
    }

    switch (*tok_op->str) {
    case '+': result = n + m; break;
    case '-': result = n - m; break;
    case '*': result = n * m; break;
    case '/': result = n / m; break;
    default:
        WARN2("unknown op '%s'", tok_op->str);
        return;
    }


    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dtoa(result, tok_result.str, MAX_TOK_CHAR_BUF, 10)) {
        WARN2("can't convert digit '%f' to string", result);
        return;
    }
    tok_result.type = TOK_DIGIT;

    stack_push(dentaku->cur_stack, &tok_result);
}


void
dentaku_eval_at_once(Dentaku *dentaku, char *src)
{
    Token tok_n, tok_m, tok_op, tok_result;
    char *after_pos;
    char *end_pos;

    end_pos = src + strlen(src);

    d_printf("dentaku_eval_at_once()");

    while (*src) {
        token_init(&tok_op);

        // get one token (tok_op)
        after_pos = get_token(src, &tok_op);
        if (after_pos == NULL)
            break;
        if (after_pos >= end_pos)
            break;
        src = after_pos;

        if (tok_op.type == TOK_OP) {
            token_init(&tok_n);
            token_init(&tok_m);

            // get one token (tok_m)
            after_pos = get_token(src, &tok_m);
            if (after_pos == NULL)
                break;
            if (after_pos >= end_pos)
                break;
            src = after_pos;

            // top_n - copy from top of stack
            memcpy(&tok_n, dentaku->cur_stack->top, sizeof(Token));


            dentaku_calc_op(dentaku, &tok_op, &tok_n, &tok_m);

            token_destroy(&tok_n);
            token_destroy(&tok_m);
            token_destroy(&tok_op);
        }
        else {
            // push one token
            d_printf("push! [%s]", tok_op.str);
            stack_push(dentaku->cur_stack, &tok_op);
        }
    }
}


void
dentaku_push_all_tokens(Dentaku *dentaku, char *src)
{
    Token cur_tok;
    char *after_pos;
    char *end_pos;
    end_pos = src + strlen(src);

    d_printf("dentaku_push_all_tokens()");

    while (*src) {
        token_init(&cur_tok);

        // get one token
        after_pos = get_token(src, &cur_tok);
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
dentaku_eval_stack(Dentaku *dentaku)
{
    Stack *stk = dentaku->cur_stack;
    Token tok_n, tok_m, tok_op, tok_result;

    d_printf("dentaku_eval_stack()");

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
        dentaku_eval_at_once(d, src);
        // dentaku_push_all_tokens(d, src);
        // dentaku_eval_stack(d);
        dentaku_show_result(d);
    }

    dentaku_destroy(d);


    return EXIT_SUCCESS;
}
