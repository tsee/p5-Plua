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
    lua_State *L;
  CODE:
    L = THIS->L;
    PLU_ENTER_STACKASSERT(L);
    luaL_unref(L, LUA_REGISTRYINDEX, THIS->registry_index);
    Safefree(THIS);
    PLU_LEAVE_STACKASSERT(L);


SV *
plu_table_t::get(key)
    SV *key;
  PREINIT:
    STRLEN len;
    char *str;
    int dopop;
    lua_State *L;
    PLU_dSTACKASSERT;
  CODE:
    L = THIS->L;
    PLU_ENTER_STACKASSERT(L);
    /* FIXME things other than numbers and strings as keys */
    if (SvFLAGS(key) & (SVf_IOK|SVf_NOK)) {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      lua_pushnumber(L, SvNV(key));
    }
    else {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      str = SvPV(key, len);
      lua_pushlstring(L, str, (size_t)len);
    }
    lua_gettable(L, -2);
    RETVAL = plu_luaval_to_perl(aTHX_ L, -1, &dopop);
    lua_pop(L, dopop+1);
    PLU_LEAVE_STACKASSERT(L);
  OUTPUT: RETVAL


void
plu_table_t::set_int(key, value)
    SV *key;
    SV *value;
  ALIAS:
    set_num = 1
    set_str = 2
    set_table = 3
  PREINIT:
    STRLEN len;
    char *str;
    PLU_dSTACKASSERT;
    lua_State *L;
  CODE:
    L = THIS->L;
    PLU_ENTER_STACKASSERT(L);
    /* FIXME this isn't exception-clean */
    if (SvFLAGS(key) & (SVf_IOK|SVf_NOK)) {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      lua_pushnumber(L, SvNV(key));
    }
    else {
      PLU_TABLE_PUSH_TO_STACK(*THIS);
      str = SvPV(key, len);
      lua_pushlstring(L, str, (size_t)len);
    }
    switch (ix) {
    case 0:
      lua_pushinteger(L, SvIV(value));
      break;
    case 1:
      lua_pushnumber(L, SvNV(value));
      break;
    case 2:
      str = SvPV(value, len);
      lua_pushlstring(L, str, (size_t)len);
      break;
    case 3:
      if (UNLIKELY( plu_push_table_obj(aTHX_ value) != 0 )) {
        lua_pop(L, 2); /* table and key */
        croak("Failed to convert Perl value to Lua table. Unsupported type?");
      }
      break;
    }
    lua_settable(L, -3);
    lua_pop(L, 1);
    PLU_LEAVE_STACKASSERT(L);


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
