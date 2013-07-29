#ifndef PLU_LUA_INLINE_H_
#define PLU_LUA_INLINE_H_

/* inlined bits of plu_lua.h */

#include "plu_inline.h"
#include "plu_debug.h"

/* Greatly inspired by Inline::Lua! */
/* Turns a Lua type into a Perl type and returns it.  
 * 'dopop' is set to 1 if the caller has to do a lua_pop,
 * otherwise 0.
 * The only case where this does not happen is if the value
 * is a LUA_TFUNCTION (luaL_ref() already pops it off). */
/* Returned SV is NOT mortalized */
PLU_STATIC_INLINE SV *
plu_luaval_to_perl(pTHX_ lua_State *L, int idx, int *dopop)
{
  *dopop = 1;
  switch (lua_type(L, idx)) {
  case LUA_TNIL:
    return &PL_sv_undef;
  case LUA_TBOOLEAN:
    return newSViv(lua_toboolean(L, idx));
  case LUA_TNUMBER:
    return newSVnv(lua_tonumber(L, idx));
  case LUA_TSTRING:
    return newSVpvn(lua_tostring(L, idx), lua_strlen(L, idx));
  case LUA_TTABLE:
    {
      SV *sv;
      *dopop = 0;
      if (UNLIKELY( idx != -1 )) {
        lua_pushvalue(L, idx);
        sv = plu_new_table_object_perl(aTHX_ L);
      }
      else {
        sv = plu_new_table_object_perl(aTHX_ L);
      }
      return sv;
    }
  case LUA_TFUNCTION:
    croak("Cannot convert a Lua function yet");
    /**dopop = 0;
    return func_ref(L);*/
  default:
    croak("Unknown/unsupported Lua type detected");
  }
}

/* Convert Perl SV to SOME Lua value on Lua stack */
PLU_STATIC_INLINE void
plu_push_sv(pTHX_ lua_State *L, SV * const sv)
{
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  if (SvROK(sv)) {
    if (sv_derived_from(sv, "PLua::Table")) {
      plu_table_t *tbl = (plu_table_t *)SvIV(SvRV(sv));
      PLU_TABLE_PUSH_TO_STACK(*tbl);
    }
    else {
      SV * const inner = SvRV(sv);
      svtype type = SvTYPE(inner);
      if (type == SVt_PVHV)
        plu_push_hash(aTHX_ L, (HV *)inner);
      else if (type == SVt_PVAV)
        plu_push_ary(aTHX_ L, (AV *)inner);
      else {
        croak("Unsupported Perl type/reference found '%s' "
              " while converting to Lua value", SvPV_nolen(sv));
      }
    }
  } /* end if "if it's a reference */
  else if (SvNOK(sv))
    lua_pushnumber(L, (lua_Number)SvNV(sv));
  else if (SvIOK(sv))
    lua_pushinteger(L, (lua_Integer)SvIV(sv));
  else if (SvPOK(sv)) {
    STRLEN len;
    char *str;
    str = SvPV(sv, len);
    lua_pushlstring(L, str, (size_t)len);
  }
  else {
    croak("Unsupported Perl type found '%s' "
          " while converting to Lua value", SvPV_nolen(sv));
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);
}

#endif
