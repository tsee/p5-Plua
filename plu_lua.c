#include "plu_lua.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ppport.h"

#include "plu_debug.h"
#include "plu_perl_context.h"
#include "plu_global_state.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int
plu_perl_lexical_to_int(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;

  PLU_GET_THX(L);

  /* FIXME check that it's an integer */
  ofs = (PADOFFSET)lua_tointeger(L, -1);

  if (UNLIKELY( ofs == NOT_IN_PAD )) {
    lua_pushnil(L);
  }
  else {
    SV * const tmpsv = PAD_SV(ofs);
    lua_pushinteger(L, (lua_Integer)SvIV(tmpsv));
  }

  return 1;
}

lua_State *
plu_new_lua_state(pTHX)
{
  lua_State *L;

  L= luaL_newstate();

  /* C equivalent of Lua 'perl = {}' */
  lua_newtable(L);
  lua_setfield(L, LUA_GLOBALSINDEX, "perl");

  luaL_openlibs(L);

  /* Install our Perl-interfacing functions */
  lua_getfield(L, LUA_GLOBALSINDEX, "perl");
  PLU_PUSH_THX(L);
  lua_pushcclosure(L, plu_perl_lexical_to_int, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_int");

  return L;
}

SV *
plu_get_lua_errmsg(pTHX)
{
  const char *str;
  size_t len;
  SV *errmsg;
  str = lua_tolstring(PLU_lua_int, -1, &len);
  errmsg = sv_2mortal(newSVpv(str, (STRLEN)len));
  lua_pop(PLU_lua_int, -1);
  return errmsg;
}

int
plu_call_lua_func(pTHX_ const char *lua_func_name)
{
  lua_getfield(PLU_lua_int, LUA_GLOBALSINDEX, lua_func_name);
  return lua_pcall(PLU_lua_int, 0, 0, 0);
}

int
plu_call_lua_func_via_registry(pTHX_ const int registry_idx)
{
  /* Put our Lua function on the Lua stack by
   * fetching it from the Lua registry; then execute */
  lua_rawgeti(PLU_lua_int, LUA_REGISTRYINDEX, registry_idx);
  return lua_pcall(PLU_lua_int, 0, 0, 0);
}


int
plu_compile_lua_block_or_croak(pTHX_ char *code, STRLEN len)
{
  int status;
  status = luaL_loadbuffer(PLU_lua_int, code, len, "_INLINED_LUA");
  switch (status) {
    case 0:
      break;
    case LUA_ERRMEM:
      croak("Memory allocation problem compiling Lua code. This is dire.");
      break;
    case LUA_ERRSYNTAX:
    default:
      {
        char *str;
        STRLEN len;
        SV *err = plu_get_lua_errmsg(aTHX);
        str = SvPV(err, len);
        croak("Error compiling inline Lua code: %*s", len, str);
        break;
      }
  };
  return status;
}

