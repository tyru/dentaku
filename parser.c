
#include "parser.h"
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
get_token(char *src, Token *tok, bool allow_signed, bool *error)
{
    char *after_pos = NULL;
    char tok_buf[MAX_TOK_CHAR_BUF];
    char *sign_pos = NULL;

    d_printf("get_token()");


    // set true when syntax error.
    *error = false;


    if (src == NULL)
        return NULL;

    char *incl_src = skip_space(src);
    if (incl_src == NULL) {
        d_printf("token - EOF");
        return NULL;
    }
    src = incl_src;


    switch (*src) {
    case '(':
        // d_printf("token - (");

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_LPAREN;

        src++;
        break;

    case ')':
        // d_printf("token - )");

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_RPAREN;

        src++;
        break;

    case '*':
    case '/':
        // d_printf("token - op [%c]", *src);

        tok_buf[0] = src[0];
        tok_buf[1] = '\0';
        tok->type = TOK_OP;

        src++;
        break;

    case '+':
    case '-':
        // if NOT signed digit
        if (allow_signed && isdigit(*skip_space(src + 1))) {
            // d_printf("token should be signed digit");

            sign_pos = src;

            src = skip_space(src + 1);
            /* FALLTHROUGH */
        }
        else {
            // d_printf("token - op [%c]", *src);

            tok_buf[0] = src[0];
            tok_buf[1] = '\0';
            tok->type = TOK_OP;

            src++;
            break;
        }

    default:
        if (isdigit(*src)) {
            // d_printf("token - digit [%c]", *src);

            after_pos = get_digit(src, tok_buf, MAX_TOK_CHAR_BUF);
            if (after_pos == NULL) {
                DIE("malformed digit");
            }
            src = after_pos;

            tok->type = TOK_DIGIT;

            break;
        }

        // syntax error
        WARN3("syntax error near [%c]%s", *src, src + 1);

        *error = true;
        return NULL;
    }

    // allocate just token's length
    // size_t alloc_num = (sign_pos == NULL ? 0 : 1) + strlen(tok_buf) + 1;
    // token_alloc(tok, alloc_num);

    if (sign_pos == NULL) {
        strncpy(tok->str, tok_buf, MAX_TOK_CHAR_BUF);
    }
    else {
        d_printf("sign_pos [%c]", *sign_pos);
        strncpy(tok->str, sign_pos, 1);
        strncpy(tok->str + 1, tok_buf, MAX_TOK_CHAR_BUF - 1);
    }

    d_printf("got! [%s]", tok->str);

    return src;
}
