#ifndef PLU_LUA_SYNTAX_EXT_H_
#define PLU_LUA_SYNTAX_EXT_H_

#include <EXTERN.h>
#include <perl.h>

/* Modify lcode to replace syntax such as "$x.int" with the respective
 * Lua / C-function-call code to implement the functionality. */
void plu_implement_lua_lexicals(pTHX_ SV *lcode);

/* Implement the embedded-function syntax munging. */
SV *plu_implement_embedded_lua_function(pTHX_ SV *funcname, SV *paramlist, SV *lcode);

#endif
