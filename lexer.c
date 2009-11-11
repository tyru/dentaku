#include "lexer.h"
#include "token.h"
#include "util.h"

#include <ctype.h>




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
    int dot_pos = 0;

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
    if (dot_pos > 0 && (dot_pos == 0 || (size_t)dot_pos == strlen(src) - 1)) {
        WARN("'.' at the head or tail of src");
        return NULL;
    }

    if (pos > (int)maxsize) {
        WARN("too large number");
        return NULL;
    }

    strncpy(buf, src, pos);
    buf[pos] = '\0';

    return src + pos;
}




// If EOF or syntax error: return NULL
// Otherwise, Return allocated token.
Token*
lexer_get_token(char *src, char **next_pos, bool allow_signed, bool *error)
{
    char *after_pos = NULL;
    char tok_buf[MAX_TOK_CHAR_BUF];
    char *sign_pos = NULL;
    Token *tok_result;
    TokenType tok_type;

    // Set true when syntax error.
    *error = false;

    // Return when eof.
    if (src == NULL)
        return NULL;
    char *incl_src = skip_space(src);
    if (incl_src == NULL) {
        return NULL;
    }
    src = incl_src;


    // TODO Use table
    switch (*src) {
    case '(':
        tok_type = TOK_LPAREN;
        goto save_chr_to_tok_buf;
    case ')':
        tok_type = TOK_RPAREN;
        goto save_chr_to_tok_buf;
    case '*':
        tok_type = TOK_MULTIPLY;
        goto save_chr_to_tok_buf;
    case '/':
        tok_type = TOK_DIVIDE;
        goto save_chr_to_tok_buf;
    case '^':
        tok_type = TOK_UP_ALLOW;

save_chr_to_tok_buf:
        tok_buf[0] = src[0];
        tok_buf[1] = '\0';

        src++;
        break;


    case '+':
        tok_type = TOK_PLUS;
        goto save_digit_to_tok_buf;
    case '-':
        tok_type = TOK_MINUS;

save_digit_to_tok_buf:
        // Allow '+<digit>' or '-<digit>'.
        if (allow_signed && isdigit(*skip_space(src + 1))) {
            // if NOT signed digit
            sign_pos = src;
            src = skip_space(src + 1);    // NOTE: Allow spaces between sign and digit.
            /* FALLTHROUGH */
        }
        else {
            tok_buf[0] = src[0];
            tok_buf[1] = '\0';

            src++;
            break;
        }

    default:
        if (isdigit(*src)) {
            after_pos = get_digit(src, tok_buf, MAX_TOK_CHAR_BUF);
            if (after_pos == NULL) {
                WARN2("malformed digit [%s]", src);
                *error = true;
                return NULL;
            }
            src = after_pos;

            tok_type = TOK_DIGIT;

            break;
        }

        // syntax error
        WARN3("syntax error near [%c]%s", *src, src + 1);

        *error = true;
        return NULL;
    }

    tok_result = malloc(sizeof(Token));
    if (! tok_result) {
        DIE("cannot allocate memory for Token!");
    }
    token_init(tok_result);
    size_t alloc_num = (sign_pos == NULL ? 0 : 1) + strlen(tok_buf) + 1;
    token_alloc(tok_result, alloc_num);

    tok_result->type = tok_type;

    if (sign_pos == NULL) {
        strncpy(tok_result->str, tok_buf, alloc_num);
    }
    else {
        tok_result->str[0] = sign_pos[0];
        strncpy(tok_result->str + 1, tok_buf, alloc_num - 1);
    }

    *next_pos = src;
    return tok_result;
}
