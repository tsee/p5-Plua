#ifndef PZ_PARSE_KW_H_
#define PZ_PARSE_KW_H_

/* The less there is in here, the better. */

#include <EXTERN.h>
#include <perl.h>

int pz_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr);

#endif
