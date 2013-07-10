#ifndef PLU_PARSE_KW_H_
#define PLU_PARSE_KW_H_

/* The less there is in here, the better. */

#include <EXTERN.h>
#include <perl.h>

/* Main keyword plugin hook */
int plu_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr);

#endif
