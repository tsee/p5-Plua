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
  int table_stack_offset;
  lua_State *L;
  char *keystr;
  size_t keylen;
  char tmp[32];
  SV *value_sv;
  int dopop;
  HV *RETVAL;

  L = THIS->L;
  PLU_ENTER_STACKASSERT(L);
  PLU_TABLE_PUSH_TO_STACK(*THIS);

  RETVAL = newHV();
  sv_2mortal((SV *)RETVAL);
  table_stack_offset = lua_gettop(L);

  lua_pushnil(L);  /* first key */
  while (lua_next(L, table_stack_offset) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */

    /* Prepare key */
    switch (lua_type(L, -2)) {
    case LUA_TSTRING:
      keystr = (char *)lua_tolstring(L, -2, &keylen);
      break;
    case LUA_TNUMBER:
    case LUA_TBOOLEAN:
    {
      lua_Number n = lua_tonumber(L, -2);
      sprintf(tmp, LUA_NUMBER_FMT, n);
      keylen = strlen(tmp);
      keystr = &tmp[0];
      break;
    }
    default:
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


AV *
plu_table_obj_to_array(pTHX_ plu_table_t *THIS, int recursive)
{
  PLU_dSTACKASSERT;
  int table_stack_offset;
  lua_State *L;
  char *keystr;
  size_t keylen;
  SV *value_sv;
  int dopop;
  AV *RETVAL;
  I32 aryidx;

  L = THIS->L;
  PLU_ENTER_STACKASSERT(L);
  PLU_TABLE_PUSH_TO_STACK(*THIS);

  RETVAL = newAV();
  sv_2mortal((SV *)RETVAL);
  table_stack_offset = lua_gettop(L);

  lua_pushnil(L);  /* first key */
  while (lua_next(L, table_stack_offset) != 0) {
    /* uses 'key' (at index -2) and 'value' (at index -1) */

    /* Prepare key - cast to int if need be */
    switch (lua_type(L, -2)) {
    case LUA_TSTRING:
    {
      SV *tmpsv;
      keystr = (char *)lua_tolstring(L, -2, &keylen);
      /* Using SV is not efficient, but may cause the perl warnings we want.
       * That in turn may cause Perl code to be run that can throw exceptions.
       * So we need to mortalize. Grmpf. */
      tmpsv = newSVpvn(keystr, (STRLEN)keylen);
      sv_2mortal(tmpsv);
      aryidx = (I32)SvIV(tmpsv);
      SvREFCNT_dec(tmpsv);
      break;
    }
    case LUA_TNUMBER:
    {
      lua_Number n = lua_tonumber(L, -2); /* Don't change its type with lua_tointeger! */
      aryidx = (I32)n; /* FIXME should this warn for actual truncation? */
      break;
    }
    case LUA_TBOOLEAN:
      aryidx = lua_toboolean(L, -2);
      break;
    default:
      croak("Unsupported Lua type '%s' for Perl array indexes", lua_typename(L, lua_type(L, 02)));
    }

    /* Prepare value */
    value_sv = plu_luaval_to_perl(aTHX_ L, -1, &dopop);
    if (recursive && SvROK(value_sv)
        && sv_derived_from(value_sv, "PLua::Table"))
    {
      AV *tmpa;
      tmpa = plu_table_obj_to_array(aTHX_ (plu_table_t *)SvIV(SvRV(value_sv)), recursive);
      SvREFCNT_dec(value_sv);
      value_sv = newRV_inc((SV *)tmpa);
    }

    (void)av_store(RETVAL, aryidx, value_sv);

    /* removes 'value' if not already done; keeps 'key' for next iteration */
    if (dopop)
      lua_pop(L, 1);
  }
  lua_pop(L, 1);

  PLU_LEAVE_STACKASSERT(L);

  return RETVAL;
}
