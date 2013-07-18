#include "plu_table.h"

#include "plu_debug.h"
#include "plu_lua.h"

plu_table_t *
plu_new_table_object(pTHX_ lua_State *ls)
{
  plu_table_t *t;
  Newx(t, 1, plu_table_t);
  t->L = ls;
  t->registry_index = luaL_ref(ls, LUA_REGISTRYINDEX);
  return t;
}

SV *
plu_new_table_object_perl(pTHX_ lua_State *ls)
{
  SV *sv;
  plu_table_t *tbl = plu_new_table_object(aTHX_ ls);
  sv = newSV(0);
  sv_setref_pv( sv, "PLua::Table", (void*)tbl );
  return sv;
}

HV *
plu_table_obj_to_hash(pTHX_ plu_table_t *THIS, int recursive)
{
  PLU_dSTACKASSERT;
  int table_offset;
  lua_State *L;
  int key_type;
  char *keystr;
  size_t keylen;
  char tmp[32];
  SV *value_sv;
  int dopop;
  HV *RETVAL;

  RETVAL = newHV();
  sv_2mortal((SV *)RETVAL);
  L = THIS->L;
  PLU_ENTER_STACKASSERT(L);
  PLU_TABLE_PUSH_TO_STACK(*THIS);
  table_offset = lua_gettop(L);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, table_offset) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */

    /* Prepare key */
    key_type = lua_type(L, -2);
    if (key_type == LUA_TSTRING) {
      keystr = (char *)lua_tolstring(L, -2, &keylen);
    }
    else if (key_type == LUA_TNUMBER || key_type == LUA_TBOOLEAN) {
      lua_Number n = lua_tonumber(L, -2);
      sprintf(tmp, LUA_NUMBER_FMT, n);
      keylen = strlen(tmp);
      keystr = &tmp[0];
    }
    else {
      croak("Unsupported Lua type '%s' for Perl hash keys", lua_typename(L, lua_type(L, 02)));
    }

    /* Prepare value */
    value_sv = plu_luaval_to_perl(aTHX_ L, -1, &dopop);
    if (recursive && SvROK(value_sv)
        && sv_derived_from(value_sv, "PLua::Table"))
    {
      HV *tmph;
      tmph = plu_table_obj_to_hash(aTHX_ (plu_table_t *)SvIV(SvRV(value_sv)), recursive);
      SvREFCNT_dec(value_sv);
      value_sv = newRV_inc((SV *)tmph);
    }

    (void)hv_store(RETVAL, keystr, keylen, value_sv, 0);

    /* removes 'value' if not already done; keeps 'key' for next iteration */
    if (dopop)
      lua_pop(L, 1);
  }
  lua_pop(L, 1);
  PLU_LEAVE_STACKASSERT(L);

  return RETVAL;
}
