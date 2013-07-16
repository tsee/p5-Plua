#include "plu_table.h"

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

