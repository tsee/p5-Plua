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

    /* Push table and key to stack */
    PLU_TABLE_PUSH_TO_STACK(*THIS);
    plu_push_sv(aTHX_ L, key);
    PLU_LEAVE_STACKASSERT_MODIFIED(L, 2); /* table + key */

    /* Fetch value, replacing key */
    lua_gettable(L, -2);
    PLU_LEAVE_STACKASSERT_MODIFIED(L, 2); /* table + value */

    /* Convert value */
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
    set = 4
  PREINIT:
    STRLEN len;
    char *str;
    PLU_dSTACKASSERT;
    lua_State *L;
  CODE:
    /* TODO generic "set" with auto-convert! */
    L = THIS->L;
    PLU_ENTER_STACKASSERT(L);
    /* FIXME this isn't exception-clean */

    /* Push table & key */
    PLU_TABLE_PUSH_TO_STACK(*THIS);
    plu_push_sv(aTHX_ L, key);
    PLU_LEAVE_STACKASSERT_MODIFIED(L, 2); /* table + key */

    /* Push value */
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
    default:
    case 4:
      plu_push_sv(aTHX_ L, value);
      break;
    }

    PLU_LEAVE_STACKASSERT_MODIFIED(L, 3);
    lua_settable(L, -3);
    lua_pop(L, 1);
    PLU_LEAVE_STACKASSERT(L);


HV *
plu_table_t::to_hash(int recursive = 0)
  CODE:
    /* FIXME check for reference loops in Lua tables? */
    RETVAL = plu_table_obj_to_hash(aTHX_ THIS, recursive);
  OUTPUT: RETVAL


AV *
plu_table_t::to_array(int recursive = 0)
  CODE:
    /* FIXME check for reference loops in Lua tables? */
    RETVAL = plu_table_obj_to_array(aTHX_ THIS, recursive);
  OUTPUT: RETVAL


AV *
plu_table_t::to_array_shifted(int recursive = 0)
  CODE:
    RETVAL = plu_table_obj_to_array(aTHX_ THIS, recursive);
    {
      SV *tmp = av_shift(RETVAL);
      if (UNLIKELY( SvOK(tmp) ))
        SvREFCNT_dec(tmp);
    }
  OUTPUT: RETVAL


size_t
plu_table_t::objlen()
  PREINIT:
    PLU_dSTACKASSERT;
    lua_State *L;
  CODE:
    L = THIS->L;
    PLU_ENTER_STACKASSERT(L);
    PLU_TABLE_PUSH_TO_STACK(*THIS);
    RETVAL = lua_objlen(L, -1);
    lua_pop(L, 1);
    PLU_LEAVE_STACKASSERT(L);
  OUTPUT: RETVAL


AV *
plu_table_t::keys()
  CODE:
    RETVAL = plu_table_obj_to_keys_array(aTHX_ THIS);
  OUTPUT: RETVAL


AV *
plu_table_t::values()
  CODE:
    RETVAL = plu_table_obj_to_values_array(aTHX_ THIS);
  OUTPUT: RETVAL


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
