
#include "dentaku-stack.h"

#include "dentaku.h"
#include "util.h"
#include "parser.h"





// get one token from dentaku->src or dentaku->data_stack->top.
static void
dentaku_push_got_token(Dentaku *dentaku)
{
    bool syntax_error;
    Token tok_result;
    bool allow_signed;
    Token top;
    stack_t *stk = dentaku->data_stack;

    if (dentaku_src_eof(dentaku))
        return;

    stack_top(stk, &top);
    // allow '+' or '-' before digit
    // when stack is empty or '(' is on the top.
    allow_signed = stack_empty(stk) || top.type == TOK_LPAREN;

    token_init(&tok_result);
    token_alloc(&tok_result, MAX_TOK_CHAR_BUF);

    char *cur_pos = dentaku->src + dentaku->src_pos;
    char *next_pos = parser_get_token(cur_pos, &tok_result, allow_signed, &syntax_error);

    if (next_pos == NULL) {
        // EOF or syntax error.
        token_destroy(&tok_result);

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
        dentaku_printf_d(dentaku, "got! [%s]", tok_result.str);

        dentaku->src_pos += next_pos - cur_pos;
        stack_push(stk, &tok_result);
    }
}


static void
eval_when_eof_or_rparen(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    bool top_op_is_mul_div = false;    // continue evaluating when op is '*' or '/'.
    Token top_buf, result;
    const TokenType top_type = (dentaku_src_eof(dentaku) ? TOK_UNDEF : TOK_RPAREN);
    const char *top_type_str = top_type == TOK_RPAREN ? "')'" : "eof";

    if (top_type == TOK_RPAREN) {
        // pop ')'
        stack_pop(stk, NULL);
    }
    dentaku_printf_d(dentaku, "top is %s", top_type_str);

    // calculate until expression becomes 1 token
    // or top is correspond '(' and next is neither '*' nor '/'.
    for (; ; token_destroy(&result)) {
        dentaku_calc_expr(dentaku);
        stack_pop(stk, &result);
        dentaku_printf_d(dentaku, "result [%s]", result.str);

        stack_ret ret = stack_top(stk, &top_buf);
        dentaku_printf_d(dentaku,
                "dispatched %s: top is %s.",
                top_type_str,
                ret == STACK_SUCCESS ? top_buf.str : "empty");


        if (stack_empty(stk)) {
            if (top_type == TOK_RPAREN && ! top_op_is_mul_div) {
                // token was ')' at first, but got EOF.
                token_destroy(&result);
                WARN("extra close parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }
            stack_push(stk, &result);

            if (dentaku_src_eof(dentaku)) {
                // end.
                token_destroy(&result);
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
                token_destroy(&result);
                token_destroy(&top_buf);
                WARN("extra open parenthesis");
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
            }

            // pop '('
            stack_pop(stk, NULL);

            // unless top is '*' or '/',
            // finish calculation of one expression in parenthesis.
            // because I got correspond '('.
            token_destroy(&top_buf);
            stack_top(stk, &top_buf);
            bool mul_or_div = ! stack_empty(stk)
                            && (top_buf.str[0] == '*'
                            ||  top_buf.str[0] == '/'
                            ||  top_buf.str[0] == '^');
            bool is_digit = ! stack_empty(stk)
                           && top_buf.type == TOK_DIGIT;

            if (stack_empty(stk) && dentaku_src_eof(dentaku)) {
                dentaku_printf_d(dentaku,
                        "no more tokens. just push the result [%s].",
                        result.str);

                stack_push(stk, &result);
                token_destroy(&result);
                siglongjmp(*dentaku->main_jmp_buf, JMP_RET_OK);
            }
            else if (top_type == TOK_RPAREN && is_digit) {
                dentaku_printf_d(dentaku, "add '*' between '(...)' and '(...)'");

                // push "*"
                Token tok_mul;
                tok_mul.str = "*";
                tok_mul.type = TOK_OP;
                stack_push(stk, &tok_mul);
                // push result.
                stack_push(stk, &result);

                token_destroy(&result);
                // forbid empty stack with top_type == TOK_RPAREN.
                // because '1+2)' is not valid expression.
                // set this value to through the checking.
                top_op_is_mul_div = true;
                continue;
            }
            else if (top_type == TOK_RPAREN && mul_or_div) {
                dentaku_printf_d(dentaku,
                        "next op is '*' or '/' or '^'. continue evaluating...");

                // same as above.
                stack_push(stk, &result);

                top_op_is_mul_div = true;
                token_destroy(&result);
                continue;
            }
            else {
                stack_push(stk, &result);
                token_destroy(&result);
                break;
            }
        }
        else if (top_buf.type == TOK_OP) {
            stack_push(stk, &result);
            if (top_buf.str[0] == '+' || top_buf.str[0] == '-')
                break;
        }
        else
            stack_push(stk, &result);
    }
    token_destroy(&result);
    token_destroy(&top_buf);
}


static void
eval_when_mul_or_div(Dentaku *dentaku)
{
    stack_t *stk = dentaku->data_stack;
    Token tok_got;

    // get and push right hand operand.
    dentaku_push_got_token(dentaku);
    // TODO I want something like alloca().
    stack_top(stk, &tok_got);


    switch (tok_got.type) {
    case TOK_DIGIT:
        dentaku_calc_expr(dentaku);
        break;

    // just syntax checking
    case TOK_RPAREN:
        WARN("reaching ')' where expression is expected");
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    case TOK_OP:
        WARN2("reaching '%c' where expression is expected", *tok_got.str);
        token_destroy(&tok_got);
        siglongjmp(*dentaku->main_jmp_buf, JMP_RET_ERR);
    default:
        token_destroy(&tok_got);
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
 * TOK_OP:
 *  - if op is '*' or '/'
 *    - get token
 *    - if it is digit, call dentaku_calc_expr()
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
        dentaku_push_got_token(dentaku);
        stack_top(stk, &tok_got);
        dentaku_printf_d(dentaku, "dispatch! [%s]", tok_got.str);

        if (dentaku_src_eof(dentaku) || tok_got.type == TOK_RPAREN) {    // EOF or ')'
            eval_when_eof_or_rparen(dentaku);
        }
        else if (tok_got.type == TOK_OP) {    // '+', '-', '*', '/'
            // postpone '+' and '-'.
            if (tok_got.str[0] == '*' || tok_got.str[0] == '/') {
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
