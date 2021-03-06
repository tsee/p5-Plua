#include "plu_lua.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ppport.h"

#include "plu_debug.h"
#include "plu_perl_context.h"
#include "plu_global_state.h"
#include "plu_table.h"
#include "plu_lua_function.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Convert lexical Perl SV to lua_Integer on Lua stack */
static int
S_plu_perl_lexical_to_integer(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);
  PLU_GET_THX(L);

  ofs = (PADOFFSET)lua_tointeger(L, -1);

  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */
  {
    SV * const tmpsv = PAD_SV(ofs);
    lua_pushinteger(L, (lua_Integer)SvIV(tmpsv));
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);

  return 1;
}


/* Convert lexical Perl SV to lua_Number on Lua stack */
static int
S_plu_perl_lexical_to_number(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  PLU_GET_THX(L);

  ofs = (PADOFFSET)lua_tointeger(L, -1);

  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */
  {
    SV * const tmpsv = PAD_SV(ofs);
    lua_pushnumber(L, (lua_Number)SvNV(tmpsv));
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);
  return 1;
}


/* Convert lexical Perl SV to lua_Number on Lua stack */
static int
S_plu_perl_lexical_to_string(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  PLU_GET_THX(L);

  ofs = (PADOFFSET)lua_tointeger(L, -1);

  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */
  {
    STRLEN len;
    char *str;
    SV * const tmpsv = PAD_SV(ofs);
    str = SvPV(tmpsv, len);
    lua_pushlstring(L, str, (size_t)len);
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);
  return 1;
}

/* Convert lexical Perl SV to a Lua table on Lua stack */
static int
S_plu_perl_lexical_to_table(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);
  PLU_GET_THX(L);

  ofs = (PADOFFSET)lua_tointeger(L, -1);

  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */
  {
    SV * const tmpsv = PAD_SV(ofs);
    if (SvROK(tmpsv) && sv_derived_from(tmpsv, "PLua::Table")) {
      plu_table_t *tbl = (plu_table_t *)SvIV(SvRV(tmpsv));
      if (tbl->L != L) {
        /* FIXME this check might run afoul of the Lua coroutine madness that is abusing
         *       lua_State* to represent both interpreters and coroutines in C. */
        luaL_error(L, "Trying to pass Lua table (a PLua::Table object) owned "
                      "by Lua interpreter '%p' to the currently executing Lua interpreter '%p'. "
                      "Lua tables are bound to their Lua interpreter of origin",
                      tbl->L, L);
      }
      PLU_TABLE_PUSH_TO_STACK(*tbl);
    }
    else {
      croak("Unsupported Perl type found '%s' "
            " while converting to Lua value", SvPV_nolen(tmpsv));
    }
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);

  return 1;
}

/* Convert lexical Perl SV to SOME Lua value on Lua stack */
static int
S_plu_perl_lexical_to_luaval(lua_State *L)
{
  PADOFFSET ofs;
  PLU_dTHX;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);
  PLU_GET_THX(L);

  ofs = (PADOFFSET)lua_tointeger(L, -1);

  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */

  plu_push_sv(aTHX_ L, PAD_SV(ofs));

  PLU_LEAVE_STACKASSERT_MODIFIED(L, 1);
  return 1;
}


static int
S_plu_lua_to_perl_lexical(lua_State *L)
{
  PADOFFSET ofs;
  int ltype;
  PLU_dTHX;
  SV *sv;
  lua_State *real_lua_int;
  PLU_dSTACKASSERT;

  PLU_ENTER_STACKASSERT(L);

  PLU_GET_THX(L);
  /* Get the real Lua interpreter (may differ from L if
   * we're executing in a co-routine) */
  real_lua_int = lua_tothread(L, lua_upvalueindex(PLU_N_THX_ARGS+1));

  ofs = (PADOFFSET)lua_tointeger(L, -2);
  /* NOT_IN_PAD should have been caught at compile time, so
   * skip checking that here. */

  sv = PAD_SVl(ofs);
  ltype = lua_type(L, -1);
  switch (ltype) {
  case LUA_TNUMBER:
    sv_setnv_mg(sv, lua_tonumber(L, -1));
    lua_pop(L, 1);
    break;
  case LUA_TBOOLEAN:
    sv_setiv_mg(sv, lua_toboolean(L, -1));
    lua_pop(L, 1);
    break;
  case LUA_TSTRING:
    {
      size_t len;
      const char *str = lua_tolstring(L, -1, &len);
      sv_setpvn_mg(sv, str, (STRLEN)len);
      lua_pop(L, 1);
      break;
    }
  case LUA_TNIL:
    sv_setsv_mg(sv, &PL_sv_undef);
    lua_pop(L, 1);
    break;
  case LUA_TTABLE:
    SvREFCNT_dec(sv);
    PAD_SVl(ofs) = plu_new_table_object_perl(aTHX_ real_lua_int);
    break;
  case LUA_TFUNCTION:
    SvREFCNT_dec(sv);
    PAD_SVl(ofs) = plu_new_function_object_perl(aTHX_ real_lua_int);
    break;
  case LUA_TUSERDATA:
  case LUA_TTHREAD:
  case LUA_TLIGHTUSERDATA:
  case LUA_TNONE:
  default:
    lua_pop(L, 1);
    PLU_LEAVE_STACKASSERT_MODIFIED(L, -1);
    luaL_error(L, "Lua variables of type '%s' currently "
                  "cannot be converted to Perl scalars", lua_typename(L, ltype));
    break;
  }

  PLU_LEAVE_STACKASSERT_MODIFIED(L, -1);
  return 0;
}


lua_State *
plu_new_lua_state(pTHX)
{
  lua_State *L;
  PLU_dSTACKASSERT;

  L= luaL_newstate();

  if (!L) {
      PerlIO_printf(Perl_debug_log, "Couldn't allocate memory for lua\n");
      return NULL;
  }

  PLU_ENTER_STACKASSERT(L);
  /* C equivalent of Lua 'perl = {}' */
  lua_newtable(L);
  lua_setfield(L, LUA_GLOBALSINDEX, "perl");

  luaL_openlibs(L);

  /* Install our Perl-interfacing functions */
  lua_getfield(L, LUA_GLOBALSINDEX, "perl");

  PLU_PUSH_THX(L);
  lua_pushcclosure(L, S_plu_perl_lexical_to_integer, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_int");

  PLU_PUSH_THX(L);
  lua_pushcclosure(L, S_plu_perl_lexical_to_number, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_num");

  PLU_PUSH_THX(L);
  lua_pushcclosure(L, S_plu_perl_lexical_to_string, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_str");

  PLU_PUSH_THX(L);
  lua_pushcclosure(L, S_plu_perl_lexical_to_table, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_table");

  PLU_PUSH_THX(L);
  lua_pushcclosure(L, S_plu_perl_lexical_to_luaval, PLU_N_THX_ARGS);
  lua_setfield(L, -2, "var_to_luaval");

  PLU_PUSH_THX(L);
  /* push ptr to the REAL interpreter lua_State for wrapping functions and tables */
  lua_pushthread(L);
  lua_pushcclosure(L, S_plu_lua_to_perl_lexical, PLU_N_THX_ARGS+1);
  lua_setfield(L, -2, "lua_val_to_sv");

  lua_pop(L, 1); /* pop off "perl" namespace table */

  PLU_LEAVE_STACKASSERT(L);

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
  status = luaL_loadbuffer(PLU_lua_int, code, len, "inlined Lua block");
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


/* Inline::Lua inspired! Push a Perl hash onto the Lua stack */
void
plu_push_hash(pTHX_ lua_State *L, HV *hv)
{
  HE* he;
  I32 len;
  char *key;

  lua_newtable(L);
  hv_iterinit(hv);

  while ((he = hv_iternext(hv))) {
    key = hv_iterkey(he, &len);
    lua_pushlstring(L, key, len);
    plu_push_sv(aTHX_ L, hv_iterval(hv, he));
    lua_settable(L, -3);
  }
}


/* Inline::Lua inspired! Push a Perl array onto the Lua stack */
void
plu_push_ary(pTHX_ lua_State *L, AV *av)
{
  lua_newtable(L);
  size_t i, n;
  n = (size_t)av_len(av)+1;

  for (i = 0; i < n; i++) {
    SV **ptr = av_fetch(av, (IV)i, FALSE);
    lua_pushnumber(L, (lua_Number)i+1);
    if (LIKELY( ptr != NULL ))
      plu_push_sv(aTHX_ L, *ptr);
    else
      lua_pushnil(L);
    lua_settable(L, -3);
  }
}


int
plu_push_table_obj(pTHX_ SV *sv)
{
  if (UNLIKELY( !sv_isobject(sv) || !sv_derived_from(sv, "PLua::Table") ))
  {
    return 1;
  }
  plu_table_t *tbl = (plu_table_t *)SvIV((SV*)SvRV(sv));
  PLU_TABLE_PUSH_TO_STACK(*tbl);
  return 0;
}

