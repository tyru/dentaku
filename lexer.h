#ifndef DENTAKU_LEXER_H
#define DENTAKU_LEXER_H

#include "common.h"



Token*
lexer_get_token(char *src, char **next_pos, bool allow_signed, bool *error);


#endif /* DENTAKU_LEXER_H */
