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
"+"     return YYTOK_ADD;
"-"     return YYTOK_SUB;
"*"     return YYTOK_MUL;
"("     return YYTOK_LP;
")"     return YYTOK_RP;
"/"     return YYTOK_DIV;
"\n"    return YYTOK_NL;
([1-9][0-9]*)|([0-9]*\.[0-9]*)|0 {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.double_value = temp;
    return DOUBLE_LITERAL;
}
%%
