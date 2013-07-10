#include "pz_lua.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pz_debug.h"
#include "pz_global_state.h"

#include "lua.h"

static SV *
pz_get_lua_errmsg(pTHX)
{
  const char *str;
  size_t len;
  SV *errmsg;
  str = lua_tolstring(PZ_lua_int, -1, &len);
  errmsg = sv_2mortal(newSVpv(str, (STRLEN)len));
  lua_pop(PZ_lua_int, -1);
  return errmsg;
}

int
pz_call_lua_func(pTHX_ const char *lua_func_name)
{
  lua_getglobal(PZ_lua_int, lua_func_name);
  return lua_pcall(PZ_lua_int, 0, 0, 0);
}


int
pz_compile_lua_block_or_croak(pTHX_ char *code, STRLEN len)
{
  int status;
  status = luaL_loadbuffer(PZ_lua_int, code, len, "_INLINED_LUA");
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
        SV *err = pz_get_lua_errmsg(aTHX);
        str = SvPV(err, len);
        croak("Error compiling inline Lua code: %*s", len, str);
        break;
      }
  };
  return status;
}

