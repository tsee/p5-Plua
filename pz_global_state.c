#include "pz_global_state.h"
#include "pz_debug.h"

#include "pz_parse_kw.h"

XOP PJ_xop;
/* Perl_ophook_t PJ_orig_opfreehook; */

/* For chaining the actual keyword plugin */
int (*PZ_next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

void
pz_init_global_state(pTHX)
{
  PZ_next_keyword_plugin = PL_keyword_plugin;
  PL_keyword_plugin = pz_my_keyword_plugin;
  /* Setup our callback for cleaning up OPs during global cleanup */
  /* PZ_orig_opfreehook = PL_opfreehook;
  PL_opfreehook = pz_free_hook;*/

  /* Setup our custom op */
  /*XopENTRY_set(&PZ_xop, xop_name, "jitop");
  XopENTRY_set(&PZ_xop, xop_desc, "a just-in-time compiled composite operation");
  XopENTRY_set(&PZ_xop, xop_class, OA_LISTOP);
  Perl_custom_op_register(aTHX_ pj_pp_jit, &PZ_xop);*/

  /* Register super-late global cleanup hook for global state */
  /* Perl_call_atexit(aTHX_ pz_global_state_final_cleanup, NULL); */
}

/* End-of-global-destruction cleanup hook.
 * Actually installed in BOOT XS section. */
/*void
pz_global_state_final_cleanup(pTHX_ void *ptr)
{
  (void)ptr;
  PZ_DEBUG("pz_final_cleanup after global destruction.\n");
}
*/
