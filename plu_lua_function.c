#include "plu_lua_function.h"

/* For versions of ExtUtils::ParseXS > 3.04_02, we need to
 * explicitly enforce exporting of XSUBs since we want to
 * refer to them using XS(). This isn't strictly necessary,
 * but it's by far the simplest way to be backwards-compatible.
 * This is for Lua functions.
 */
#define PERL_EUPXS_ALWAYS_EXPORT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
/* Note: Class::XSAccessor defines PERL_CORE in scope of XSUB.h inclusion as an optimization.
 *       except on Win32. TODO: Find out whether that optimization can be cribbed. */

#include "plu_debug.h"
#include "plu_lua.h"

plu_function_t *
plu_new_function_object(pTHX_ lua_State *ls)
{
  plu_function_t *func;
  int func_id;
  PLU_dSTACKASSERT;
  PLU_ENTER_STACKASSERT(ls);

  func_id = luaL_ref(ls, LUA_REGISTRYINDEX);

  Newx(func, 1, plu_function_t);
  func->L = ls;
  func->registry_index = func_id;

  PLU_LEAVE_STACKASSERT_MODIFIED(ls, -1);
  return func;
}

/* Create a new XSUB instance and set it's XSANY.any_ptr to contain the
 * necessary user data.
 * Requires a previous declaration of a CV* cv!
 **/
#define NEW_CV_WITH_PTR(xsub, user_pointer)                                 \
STMT_START {                                                                \
  cv = newXS(NULL, xsub, (char*)__FILE__);                                  \
  if (cv == NULL)                                                           \
    croak("ARG! Something went really wrong while installing a new XSUB!"); \
  XSANY.any_ptr = (void *)user_pointer;                                     \
} STMT_END


/* Predeclare so we can reference the callback function template */
XS(XS_PLua__invoke_lua_function);

/* Creates and returns a blessed subreference */
SV *
plu_new_function_object_perl(pTHX_ lua_State *L)
{
  SV *rv;
  CV *cv;
  HV *stash;
  plu_function_t *func;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  stash = gv_stashpvs("PLua::Function", 1);
  func = plu_new_function_object(aTHX_ L);

  NEW_CV_WITH_PTR(XS_PLua__invoke_lua_function, func);
  rv = newRV_noinc((SV *)cv);
  sv_bless(rv, stash);

  PLU_LEAVE_STACKASSERT_MODIFIED(L, -1);
  return rv;
}

#undef NEW_CV_WITH_PTR
