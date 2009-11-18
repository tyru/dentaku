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
"%"     return DENTAKU_MOD;
"("     return DENTAKU_LP;
")"     return DENTAKU_RP;
"\n"    return DENTAKU_NL;
([1-9][0-9]*)|([0-9]*\.[0-9]*)|0 {
    Token tok;
    token_init(&tok);
    const size_t alloc_byte = strlen(yytext) + 1;
    token_alloc(&tok, alloc_byte);
    strncpy(tok.str, yytext, alloc_byte);

    token2digit(&yylval.digit, &tok, 10);
    return DOUBLE_LITERAL;
}
%%
