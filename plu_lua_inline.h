#ifndef PLU_LUA_INLINE_H_
#define PLU_LUA_INLINE_H_

/* inlined bits of plu_lua.h */

#include "plu_inline.h"

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
      if (UNLIKELY( idx != -1 )) {
        lua_pushvalue(L, idx);
        sv = plu_new_table_object_perl(aTHX_ L);
        lua_pop(L, 1);
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

#endif
