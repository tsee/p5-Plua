#ifndef PLU_GLOBAL_STATE_H_
#define PLU_GLOBAL_STATE_H_

/* The less there is in here, the better. */

#include <EXTERN.h>
#include <perl.h>

#include <lua.h>

/* The Lua compiler/interpreter - FIXME ought to be in thread-local storage */
extern lua_State *PLU_lua_int;

/* The custom op definition structure */
extern XOP PLU_xop;

/* For chaining the actual keyword plugin */
extern int (*PLU_next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

/* Original opfreehook - we wrap this to free JIT OP aux structs */
extern Perl_ophook_t PLU_orig_opfreehook;

/* Initialize global state like custom op description, etc. */
void plu_init_global_state(pTHX);

/* End-of-global-destruction cleanup hook. */
void plu_global_state_final_cleanup(pTHX_ void *ptr);


#endif
