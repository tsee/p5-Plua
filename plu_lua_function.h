#ifndef PLU_LUA_FUNCTION_H_
#define PLU_LUA_FUNCTION_H_

#include <EXTERN.h>
#include <perl.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct plu_function {
  lua_State *L;
  int registry_index;
} plu_function_t;

/* Create new function object from Lua function on top of Lua stack, pops function off of stack */
plu_function_t *plu_new_function_object(pTHX_ lua_State *ls);

/* Same as plu_new_function_object, but then converts it to a
 * Perl sub{} (RV to CV) (not mortalized) */
SV *plu_new_function_object_perl(pTHX_ lua_State *ls);

#define PLU_LUA_FUNCTION_PUSH_TO_STACK(func) \
    lua_rawgeti((func).L, LUA_REGISTRYINDEX, (func).registry_index)

#endif
