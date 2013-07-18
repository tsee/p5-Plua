#ifndef PLU_DEBUG_H_
#define PLU_DEBUG_H_

/* Set up debugging output to be compiled out without assertions */

#ifdef NDEBUG
#  define PLU_DEBUGGING 0
#  define PLU_DEBUG (void)
#  define PLU_DEBUG_1 (void)
#  define PLU_DEBUG_2 (void)
#  define PLU_dSTACKASSERT (void)
#  define PLU_ENTER_STACKASSERT(L) (void)
#  define PLU_LEAVE_STACKASSERT(L) (void)
#  define PLU_LEAVE_STACKASSERT_MODIFIED(L, change) (void)
#else
#  define PLU_DEBUGGING 1
#  define PLU_DEBUG(s) printf(s)
#  define PLU_DEBUG_1(s, par1) printf(s, par1)
#  define PLU_DEBUG_2(s, par1, par2) printf(s, par1, par2)
#  define PLU_dSTACKASSERT \
    int stackassertelems; \
    char * stackassertfile; \
    int stackassertline
#  define PLU_ENTER_STACKASSERT(L) \
  STMT_START { \
    stackassertelems = lua_gettop((L)); \
    stackassertfile = __FILE__; \
    stackassertline = __LINE__; \
  } STMT_END
#  define PLU_LEAVE_STACKASSERT(L) \
    PLU_LEAVE_STACKASSERT_MODIFIED(L, 0)
#  define PLU_LEAVE_STACKASSERT_MODIFIED(L, change) \
  STMT_START { \
    if (lua_gettop((L)) != stackassertelems + (change)) { \
      croak("Lua stack is unbalanced. " \
            "At start: %i elements (at %s:%i). Now: %i elements " \
            "(at %s:%i)", stackassertelems, stackassertfile, stackassertline, \
            lua_gettop((L)), __FILE__, __LINE__); \
    } \
  } STMT_END
#endif

#endif
