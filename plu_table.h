#ifndef PLU_TABLE_H_
#define PLU_TABLE_H_

#include <EXTERN.h>
#include <perl.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct plu_table {
  lua_State *L;
  int registry_index;
} plu_table_t;

/* Create new table object from table on top of Lua stack, pops table off of stack */
plu_table_t *plu_new_table_object(pTHX_ lua_State *ls);

/* Same as plu_new_table_object, but then converts it to a
 * blessed Perl SV (not mortalized) */
SV *plu_new_table_object_perl(pTHX_ lua_State *ls);

/* Convert the guts of an XS wrapper of a Lua table to a (mortal) Perl HV */
HV *plu_table_obj_to_hash(pTHX_ plu_table_t *THIS, int recursive);

/* Convert the guts of an XS wrapper of a Lua table to a (mortal) Perl AV */
AV *plu_table_obj_to_array(pTHX_ plu_table_t *THIS, int recursive);

#define PLU_TABLE_PUSH_TO_STACK(tbl) \
    lua_rawgeti((tbl).L, LUA_REGISTRYINDEX, (tbl).registry_index)

#endif
