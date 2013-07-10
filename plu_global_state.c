#include "plu_global_state.h"
#include "plu_debug.h"
#include "plu_inline.h"

#include "plu_parse_kw.h"
#include "plu_op.h"
#include "plu_lua.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* The Lua compiler/interpreter - FIXME ought to be in thread-local storage */
lua_State *PLU_lua_int = NULL;

XOP PLU_xop;
Perl_ophook_t PLU_orig_opfreehook;

/* The sequence number of all Perl/Lua binding Lua functions */
unsigned long PLU_global_lua_func_count = 27; /* let's not start at 0 to avoid easy collision */

/* For chaining the actual keyword plugin */
int (*PLU_next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

void
plu_init_global_state(pTHX)
{
  /* Setup the actual keyword plugin */
  PLU_next_keyword_plugin = PL_keyword_plugin;
  PL_keyword_plugin = plu_my_keyword_plugin;

  /* Setup our callback for cleaning up OPs during global cleanup */
  PLU_orig_opfreehook = PL_opfreehook;
  PL_opfreehook = plu_op_free_hook;

  /* Setup our custom op */
  XopENTRY_set(&PLU_xop, xop_name, "luaop");
  XopENTRY_set(&PLU_xop, xop_desc, "Inlined Lua Execution");
  XopENTRY_set(&PLU_xop, xop_class, OA_BASEOP);
  Perl_custom_op_register(aTHX_ plu_pp_custom, &PLU_xop);

  /* Init Lua compiler/interpreter */
  PLU_lua_int = plu_new_lua_state(aTHX);

  /* Register super-late global cleanup hook for global state */
  Perl_call_atexit(aTHX_ plu_global_state_final_cleanup, NULL);
}

/* End-of-global-destruction cleanup hook.
 * Actually installed in BOOT XS section. */
void
plu_global_state_final_cleanup(pTHX_ void *ptr)
{
  (void)ptr;
  /*PLU_DEBUG("plu_final_cleanup after global destruction.\n"); */
  if (PLU_lua_int != NULL) {
    lua_close(PLU_lua_int);
    PLU_lua_int = NULL;
  }
}
