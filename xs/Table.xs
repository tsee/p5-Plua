MODULE = PLua PACKAGE = PLua::Table
PROTOTYPES: DISABLE

void
plu_table_t::DESTROY()
  CODE:
    luaL_unref(THIS->L, LUA_REGISTRYINDEX, THIS->registry_index); 
    Safefree(THIS);

SV *
plu_table_t::get(key)
    SV *key;
  PREINIT:
    STRLEN len;
    char *str;
    int dopop;
  CODE:
    str = SvPV(key, len);
    PLU_TABLE_PUSH_TO_STACK(*THIS);
    lua_pushlstring(THIS->L, str, (size_t)len);
    lua_gettable(THIS->L, -2);
    RETVAL = plu_luaval_to_perl(aTHX_ THIS->L, -1, &dopop);
    lua_pop(THIS->L, 2 + dopop);
  OUTPUT: RETVAL

SV *
_make_table()
  CODE:
    /* JUST FOR TESTING */
    lua_newtable(PLU_lua_int);
    lua_pushinteger(PLU_lua_int, 42);
    lua_setfield(PLU_lua_int, -2, "foo");
    RETVAL = plu_new_table_object_perl(aTHX_ PLU_lua_int);
  OUTPUT: RETVAL
