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
 * necessary user data. fqname can be NULL => fully anonymous sub
 * Requires a previous declaration of a CV* cv!
 **/
#define NEW_CV_WITH_PTR_NAME(xsub, user_pointer, fqname)                    \
STMT_START {                                                                \
  cv = newXS(fqname, xsub, (char*)__FILE__);                                \
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
  return plu_install_new_function_object_perl(aTHX_ L, (char *)NULL);
}


SV *
plu_install_new_function_object_perl(pTHX_ lua_State *L, char *fullname)
{
  SV *rv;
  CV *cv;
  HV *stash;
  plu_function_t *func;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  stash = gv_stashpvs("PLua::Function", 1);
  func = plu_new_function_object(aTHX_ L);

  NEW_CV_WITH_PTR_NAME(XS_PLua__invoke_lua_function, func, fullname);
  /* If we're installing into the stash, then one reference will be
   * owned by it + one by the reference we're about to create.
   * I'm about 85% certain that this is correct and won't leak. */
  if (fullname != NULL)
    SvREFCNT_inc(cv);
  rv = newRV_noinc((SV *)cv);
  sv_bless(rv, stash);

  PLU_LEAVE_STACKASSERT_MODIFIED(L, -1);
  return rv;
}

#undef NEW_CV_WITH_PTR_NAME
