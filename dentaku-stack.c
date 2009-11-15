
#include "dentaku-stack.h"
#include "dentaku-core.h"
#include "util.h"
#include "lexer.h"
#include "op.h"
#include "token.h"

#include "libdatastruct/stack.h"




// Ugly.
static void
eval_stack_expr(Dentaku *dentaku)
{
    Token tok_n, tok_op, tok_m, tok_result, tmp;
    stack_t *stk = dentaku->data_stack;
    stack_ret ret;

    token_init(&tok_n);
    token_init(&tok_op);
    token_init(&tok_m);


    if (stack_empty(stk)) {
        // don't call when top is NULL!!
        DIE("no more items on stack.");
    }

    // 1st. pop 'm' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_m)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'm' of 'n <op> m'... [%s]", tok_m.str);
        break;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }

    // 2nd. pop '<op>' of expression 'n <op> m'.
    // '<op>' or '( <op>'
    switch (ret = stack_pop(stk, &tok_op)) {
    /* when '(' or empty stack, just push result (tok_m) and return */
    case STACK_SUCCESS:
        if (tok_op.type == TOK_PLUS && stack_empty(stk)) {
            goto push_result_return;
        }
        else if (tok_op.type == TOK_MINUS && stack_empty(stk)) {
            // fix for the case that '-(1+1)' results in error.

            // m -> -m
            Digit d;
            token2digit(&tok_m, &d, 10);
            op_unary_minus(&d, &d);

            // allocate for '-' and tok_m.str.
            size_t alloc_size = strlen(tok_m.str) + 2;
            token_alloc(&tok_m, alloc_size);

            digit2token(&d, &tok_m, alloc_size, 10);
            goto push_result_return;
        }
        else if (tok_op.type == TOK_LPAREN) {
            stack_push(stk, &tok_op);
            goto push_return;
        }
        else {
            break;
        }
    case STACK_EMPTY:
        goto push_result_return;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }
    dentaku_printf_d(dentaku, "pop '<op>' of 'n <op> m'... [%s]", tok_op.str);

    // 3rd. pop 'n' of expression 'n <op> m'.
    switch (ret = stack_pop(stk, &tok_n)) {
    case STACK_SUCCESS:
        dentaku_printf_d(dentaku, "pop 'n' of 'n <op> m'... [%s]", tok_n.str);
        break;
    case STACK_EMPTY:
        WARN("reaching EOF where digit is expected");
        goto error;
    default:
        DIE2("something wrong stack_pop(stk, &tok_m) == %d", ret);
    }

    if (stack_top(stk, &tmp) == STACK_SUCCESS) {
        // fix for the case that '-1 - 1 - 1' results in '-1'.
        // 1 - 1 => 0
        // -1 - 0 => -1
        if (tmp.type == TOK_MINUS) {
            // fix to '-1 + -1 - 1'
            // -1 - 1 => -2
            // -1 + -2 => -3

            // pop '-'
            stack_pop(stk, NULL);

            // push '+'
            Token tok_plus;
            tok_plus.str = "+";
            tok_plus.type = TOK_PLUS;
            stack_push(stk, &tok_plus);

            // n -> -n
            Token minus_tok_n;
            token_alloc(&minus_tok_n, strlen(tok_n.str) + 2);
            minus_tok_n.str[0] = '-';
            strcpy(minus_tok_n.str + 1, tok_n.str);
            tok_n.str = minus_tok_n.str;
        }
    }

    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);
    if (! dentaku_calc_expr(dentaku, &tok_op, &tok_n, &tok_m, &tok_result))
        goto error;

    // Push result.
    stack_push(stk, &tok_result);

    return;


push_result_return:
    stack_push(stk, &tok_m);
    siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
push_return:
    stack_push(stk, &tok_m);
    return;
error:
    siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
}


// Get one token from dentaku->src if not eof.
//
// TODO make this independant with stack_t.
// and move it to dentaku-core.c
static void
push_got_token(Dentaku *dentaku)
{
    bool syntax_error;
    Token *tok_result;
    char *next_pos;
    bool allow_signed;
    Token top;
    stack_t *stk = dentaku->data_stack;

    if (dentaku_src_eof(dentaku))
        return;

    // Allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    allow_signed = stack_top(stk, &top) == STACK_EMPTY
                || top.type == TOK_LPAREN;

    char *cur_pos = dentaku->src + dentaku->src_pos;
    tok_result = lexer_get_token(cur_pos, &next_pos, allow_signed, &syntax_error);

    if (tok_result == NULL) {    // EOF or syntax error.
        // set to EOF.
        dentaku->src_pos = dentaku->src_len;
        dentaku_printf_d(dentaku, "reach EOF");

        if (syntax_error)
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        else if (stack_empty(stk))
            // top of stack is NULL and reached EOF.
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else {
        dentaku_printf_d(dentaku, "got! [%s]", tok_result->str);

        if (TOKEN_IS_OPERATOR(top) && TOKEN_IS_OPERATOR(*tok_result)) {
            WARN3("operator continued after operator: %s %s ...",
                    top.str, tok_result->str);
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }

        dentaku->src_pos += next_pos - cur_pos;
        stack_push(stk, tok_result);
        // TODO use GC (not just pushing allocated pointers to list)
        free(tok_result);
    }
}


static void
eval_when_eof_or_rparen(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    bool allow_rparen = false;    // continue evaluating when op has higher priority
    Token top_buf, result;
    const TokenType top_type = (dentaku_src_eof(dentaku) ? TOK_UNDEF : TOK_RPAREN);
    const char *top_type_str = top_type == TOK_RPAREN ? "')'" : "eof";

    if (top_type == TOK_RPAREN) {
        // pop ')'
        stack_pop(stk, NULL);
    }
    dentaku_printf_d(dentaku, "top is %s", top_type_str);

    // calculate until expression becomes 1 token
    // or top is correspond '(' and next does not have higher priority
    while (1) {
        eval_stack_expr(dentaku);
        stack_pop(stk, &result);
        dentaku_printf_d(dentaku, "result [%s]", result.str);

        stack_ret ret = stack_top(stk, &top_buf);
        dentaku_printf_d(dentaku,
                "dispatched %s: top is %s.",
                top_type_str,
                ret == STACK_SUCCESS ? top_buf.str : "empty");


        if (stack_empty(stk)) {
            if (top_type == TOK_RPAREN && ! allow_rparen) {
                // token was ')' at first, but got EOF.
                WARN("extra close parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }
            stack_push(stk, &result);

            if (dentaku_src_eof(dentaku)) {
                // end.
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else
                // no more tokens on stack.
                // so I will get tokens from dentaku->src.
                break;
        }
        else if (top_buf.type == TOK_LPAREN) {
            if (top_type == TOK_UNDEF) {
                // token was EOF at first, but got '('.
                WARN("extra open parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }

            // pop '('
            stack_pop(stk, NULL);

            // unless top is '*' or '/',
            // finish calculation of one expression in parenthesis.
            // because I got correspond '('.
            stack_top(stk, &top_buf);
            bool is_digit = top_type == TOK_RPAREN
                         && ! stack_empty(stk)
                         && top_buf.type == TOK_DIGIT;
            bool high_priority = top_type == TOK_RPAREN
                              && ! stack_empty(stk)
                              && TOKEN_HAS_HIGHER_PRIORITY(top_buf);

            if (stack_empty(stk) && dentaku_src_eof(dentaku)) {
                dentaku_printf_d(dentaku,
                        "no more tokens. just push the result [%s].",
                        result.str);

                stack_push(stk, &result);
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else if (is_digit) {
                dentaku_printf_d(dentaku, "add '*' between '(...)' and '(...)'");

                // push "*"
                Token tok_mul;
                tok_mul.str = "*";
                tok_mul.type = TOK_MULTIPLY;
                stack_push(stk, &tok_mul);
                // push result.
                stack_push(stk, &result);

                // forbid empty stack with top_type == TOK_RPAREN.
                // because '1+2)' is not valid expression.
                // set this value to through the checking.
                allow_rparen = true;
                continue;
            }
            else if (high_priority) {
                dentaku_printf_d(dentaku,
                        "next op is '*' or '/' or '^'. continue evaluating...");

                // same as above.
                stack_push(stk, &result);

                allow_rparen = true;
                continue;
            }
            else {
                stack_push(stk, &result);
                break;
            }
        }
        else if (TOKEN_IS_OPERATOR(top_buf)) {
            stack_push(stk, &result);
            if (! TOKEN_HAS_HIGHER_PRIORITY(top_buf))
                break;
        }
        else {
            stack_push(stk, &result);
        }
    }
}


static void
eval_when_mul_or_div(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;

    // get and push right hand operand.
    push_got_token(dentaku);
    stack_top(stk, &tok_got);


    if (tok_got.type == TOK_DIGIT) {
        eval_stack_expr(dentaku);
    }
    else if (tok_got.type == TOK_RPAREN) {
        WARN("reaching ')' where expression is expected");
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
    else if (TOKEN_IS_OPERATOR(tok_got)) {
        WARN2("reaching '%c' where expression is expected", tok_got.str[0]);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    }
}





/*
 * TOK_RPAREN or EOF:
 *  - pop if TOK_RPAREN
 *  - repeat
 *    - pop top of TOK_DIGIT (m)
 *    - if top is NULL, push m and return.
 *    - pop top of TOK_LPAREN (op)
 *    - pop top of TOK_DIGIT (n)
 *    - do calculation (result)
 *    - if top is '(', pop it
 *    - push result
 * operator:
 *  - if op is '*' or '/'
 *    - get token
 *    - if it is digit, call eval_stack_expr()
 * TOK_DIGIT:
 * TOK_LPAREN:
 *  - nop
 */
NORETURN void
dentaku_stack_run(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;


    while (1) {
        push_got_token(dentaku);
        stack_top(stk, &tok_got);
        dentaku_printf_d(dentaku, "dispatch! [%s]", tok_got.str);

        if (dentaku_src_eof(dentaku) || tok_got.type == TOK_RPAREN) {    // EOF or ')'
            eval_when_eof_or_rparen(dentaku);
        }
        else if (TOKEN_IS_OPERATOR(tok_got)) {
            // postpone '+' and '-'.
            if (TOKEN_HAS_HIGHER_PRIORITY(tok_got)) {
                eval_when_mul_or_div(dentaku);
            }
        }
        else if (tok_got.type == TOK_DIGIT) {
            // nop.
        }
        else if (tok_got.type == TOK_LPAREN) {
            // nop.
        }
        else {
            if (tok_got.str)
                WARN2("unknown token '%s' found", tok_got.str);
            else
                WARN("unknown token found");
            siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
        }
    }
}
