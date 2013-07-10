#include "pz_global_state.h"
#include "pz_debug.h"

#include "pz_parse_kw.h"
#include "pz_op.h"

/* The Lua compiler/interpreter - FIXME ought to be in thread-local storage */
lua_State *PZ_lua_int = NULL;

XOP PZ_xop;
Perl_ophook_t PZ_orig_opfreehook;

/* The sequence number of all Perl/Lua binding Lua functions */
unsigned long PZ_global_lua_func_count = 27; /* let's not start at 0 to avoid easy collision */

/* For chaining the actual keyword plugin */
int (*PZ_next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

void
pz_init_global_state(pTHX)
{
  /* Setup the actual keyword plugin */
  PZ_next_keyword_plugin = PL_keyword_plugin;
  PL_keyword_plugin = pz_my_keyword_plugin;

  /* Setup our callback for cleaning up OPs during global cleanup */
  PZ_orig_opfreehook = PL_opfreehook;
  PL_opfreehook = pz_op_free_hook;

  /* Setup our custom op */
  XopENTRY_set(&PZ_xop, xop_name, "luaop");
  XopENTRY_set(&PZ_xop, xop_desc, "Inlined Lua Execution");
  XopENTRY_set(&PZ_xop, xop_class, OA_BASEOP);
  Perl_custom_op_register(aTHX_ pz_pp_custom, &PZ_xop);

  /* Init Lua compiler/interpreter */
  PZ_lua_int = luaL_newstate();
  luaL_openlibs(PZ_lua_int);

  /* Register super-late global cleanup hook for global state */
  Perl_call_atexit(aTHX_ pz_global_state_final_cleanup, NULL);
}

/* End-of-global-destruction cleanup hook.
 * Actually installed in BOOT XS section. */
void
pz_global_state_final_cleanup(pTHX_ void *ptr)
{
  (void)ptr;
  /*PZ_DEBUG("pz_final_cleanup after global destruction.\n"); */
  if (PZ_lua_int != NULL) {
    lua_close(PZ_lua_int);
    PZ_lua_int = NULL;
  }
}
