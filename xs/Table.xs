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
  CODE:
    str = SvPV(key, len);
    PLU_TABLE_PUSH_TO_STACK(*THIS);
    lua_pushlstring(THIS->L, str, (size_t)len);
    lua_gettable(THIS->L, -2);
    /* TODO: THIS NEEDS TO USE A TENTATIVE FUNCTION TO DETERMINE TARGET TYPE, see also Inline::Lua */
    RETVAL = newSViv((IV)lua_tointeger(THIS->L, -1));
    lua_pop(THIS->L, 2);
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
