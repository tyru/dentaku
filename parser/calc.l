/* via http://kmaebashi.com/programmer/devlang/yacclex.html */

%{
#include <stdio.h>
#include "y.tab.h"

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
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.double_value = temp;
    return DOUBLE_LITERAL;
}
%%
