/* via http://kmaebashi.com/programmer/devlang/yacclex.html */

%{
#include "common.h" /* to see 'Digit' for next include. */
#include "y.tab.h"

#include "token.h"
#include "util.h"

#include <string.h>

int
yywrap(void)
{
    return 1;
}
%}
%%
"+"     return DENTAKU_ADD;
"-"     return DENTAKU_SUB;
"*"     return DENTAKU_MUL;
"/"     return DENTAKU_DIV;
"^"     return DENTAKU_POW;
"("     return DENTAKU_LP;
")"     return DENTAKU_RP;
"\n"    return DENTAKU_NL;
([1-9][0-9]*)|([0-9]*\.[0-9]*)|0 {
    Token tok;
    token_init(&tok);
    token_alloc(&tok, MAX_TOK_CHAR_BUF);
    strncpy(tok.str, yytext, MAX_TOK_CHAR_BUF);
    token2digit(&tok, &yylval.digit, 10);
    return DOUBLE_LITERAL;
}
%%
