MODULE = PLua PACKAGE = PLua
PROTOTYPES: DISABLE

void
_invoke_lua_function(...)
  ALIAS:
  PREINIT:
    plu_function_t *func;
    lua_State *L;
    PLU_dSTACKASSERT;
    unsigned int i;
    int nreturn;
    int dopop;
    int lua_stacklevel;
    int call_status;
  PPCODE:
    /* TODO: Move to C function in plu_lua_function.{c,h}? */
    func = (plu_function_t *)XSANY.any_ptr;
    L = func->L;
    PLU_ENTER_STACKASSERT(L);

    /* Determine number of return values */
    if (GIMME_V == G_VOID)
      nreturn = 0;
    else {
      nreturn = LUA_MULTRET;
      lua_stacklevel = lua_gettop(L);
    }

    /* Push Lua function to stack */
    PLU_LUA_FUNCTION_PUSH_TO_STACK(*func);

    /* Convert/push all parameters to stack */
    for (i = 0; i < (unsigned int)items; ++i)
      plu_push_sv(aTHX_ L, ST(i));

    /* Call Lua function using Perl context to determine
     * the number of return values */
    call_status = lua_pcall(L, items, nreturn, 0); /* TODO: errfunc for errmsg munging? */
    if (UNLIKELY( call_status != 0 )) {
      SV *err = plu_get_lua_errmsg(aTHX);
      lua_pop(PLU_lua_int, 1);
      croak("%s", SvPVX(err));
    }

    /* Convert return values if any */
    if (nreturn != 0) { /* == 0 only in Perl void context */
      int npop = 0;
      /* determine actual # return values */
      nreturn = lua_gettop(L) - lua_stacklevel;
      for (i = 0; i < (unsigned int)nreturn; ++i) {
        SV *tmpsv = plu_luaval_to_perl(aTHX_ L, -nreturn + i, &dopop);
        npop += dopop;
        mXPUSHs(tmpsv);
      }
      if (npop)
        lua_pop(L, npop);
    }

    PLU_LEAVE_STACKASSERT(L);
    XSRETURN(nreturn);

MODULE = PLua PACKAGE = PLua::Function
PROTOTYPES: DISABLE

void
DESTROY(CV *THIS)
  PREINIT:
    PLU_dSTACKASSERT;
    lua_State *L;
    plu_function_t *func;
  CODE:
    {
      CV *cv = THIS;
      func = (plu_function_t *)XSANY.any_ptr;
    }
    PLU_DEBUG("Freeing Lua function ref\n");
    L = func->L;
    PLU_ENTER_STACKASSERT(L);
    luaL_unref(L, LUA_REGISTRYINDEX, func->registry_index);
    Safefree(func);
    PLU_LEAVE_STACKASSERT(L);
