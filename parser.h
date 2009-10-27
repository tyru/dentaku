#ifndef DENTAKU_PARSER_H
#define DENTAKU_PARSER_H

#include "common.h"

#include "token.h"



char*
parser_get_token(char *src, Token *tok, bool allow_signed, bool *error);


#endif /* DENTAKU_PARSER_H */
