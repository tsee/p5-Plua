MODULE = PLua PACKAGE = PLua::Table
PROTOTYPES: DISABLE

plu_table_t *
plu_table_t::new()
  PREINIT:
    PLU_dSTACKASSERT;
  CODE:
    PLU_ENTER_STACKASSERT(PLU_lua_int);
    lua_createtable(PLU_lua_int, 0, 0);
    RETVAL = plu_new_table_object(aTHX_ PLU_lua_int);
    PLU_LEAVE_STACKASSERT(PLU_lua_int);
  OUTPUT: RETVAL


void
plu_table_t::DESTROY()
  PREINIT:
    PLU_dSTACKASSERT;
  CODE:
    PLU_ENTER_STACKASSERT(PLU_lua_int);
    luaL_unref(THIS->L, LUA_REGISTRYINDEX, THIS->registry_index); 
    Safefree(THIS);
    PLU_LEAVE_STACKASSERT(PLU_lua_int);


SV *
plu_table_t::get(key)
    SV *key;
  PREINIT:
    STRLEN len;
    char *str;
    int dopop;
    PLU_dSTACKASSERT;
  CODE:
    PLU_ENTER_STACKASSERT(PLU_lua_int);
    /* FIXME things other than numbers and strings as keys */
    if (SvFLAGS(key) & (SVf_IOK|SVf_NOK)) {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      lua_pushnumber(THIS->L, SvNV(key));
    }
    else {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      str = SvPV(key, len);
      lua_pushlstring(THIS->L, str, (size_t)len);
    }
    lua_gettable(THIS->L, -2);
    RETVAL = plu_luaval_to_perl(aTHX_ THIS->L, -1, &dopop);
    lua_pop(THIS->L, dopop+1);
    PLU_LEAVE_STACKASSERT(PLU_lua_int);
  OUTPUT: RETVAL


void
plu_table_t::set_int(key, value)
    SV *key;
    SV *value;
  ALIAS:
    set_num = 1
    set_str = 2
  PREINIT:
    STRLEN len;
    char *str;
    PLU_dSTACKASSERT;
  CODE:
    PLU_ENTER_STACKASSERT(PLU_lua_int);
    /* FIXME this isn't exception-clean */
    if (SvFLAGS(key) & (SVf_IOK|SVf_NOK)) {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      lua_pushnumber(THIS->L, SvNV(key));
    }
    else {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      str = SvPV(key, len);
      lua_pushlstring(THIS->L, str, (size_t)len);
    }
    switch (ix) {
    case 0:
      lua_pushinteger(THIS->L, SvIV(value));
      break;
    case 1:
      lua_pushnumber(THIS->L, SvNV(value));
      break;
    case 2:
      str = SvPV(value, len);
      lua_pushlstring(THIS->L, str, (size_t)len);
      break;
    }
    lua_settable(THIS->L, -3);
    lua_pop(THIS->L, 1);
    PLU_LEAVE_STACKASSERT(PLU_lua_int);

SV *
_make_table()
  PREINIT:
    PLU_dSTACKASSERT;
  CODE:
    /* JUST FOR TESTING */
    PLU_ENTER_STACKASSERT(PLU_lua_int);
    lua_newtable(PLU_lua_int);
    lua_pushinteger(PLU_lua_int, 42);
    lua_setfield(PLU_lua_int, -2, "foo");
    RETVAL = plu_new_table_object_perl(aTHX_ PLU_lua_int);
    PLU_LEAVE_STACKASSERT(PLU_lua_int);
  OUTPUT: RETVAL
