#ifndef DENTAKU_PARSER_H
#define DENTAKU_PARSER_H

#include "common.h"



Token*
parser_get_token(char *src, char **next_pos, bool allow_signed, bool *error);


#endif /* DENTAKU_PARSER_H */
