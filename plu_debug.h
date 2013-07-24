#ifndef PLU_DEBUG_H_
#define PLU_DEBUG_H_

/* Purpose of this set of macros is to compile in a number of debug
 * facilities if NDEBUG (see assert.h) is not defined. */

/* Documentation added in the "debugging enabled" CPP branch below */

#ifdef NDEBUG
#  define PLU_DEBUGGING 0
#  define PLU_DEBUG (void)
#  define PLU_DEBUG_1 (void)
#  define PLU_DEBUG_2 (void)
#  define PLU_dSTACKASSERT (void)0
#  define PLU_ENTER_STACKASSERT(L) (void)0
#  define PLU_LEAVE_STACKASSERT(L) (void)0
#  define PLU_LEAVE_STACKASSERT_MODIFIED(L, change) (void)0
#  define PLU_DEBUG_STACKSIZE(L, id) (void)0
#else
   /* define to avoid checking NDEBUG directly */
#  define PLU_DEBUGGING 1
   /* basic debug output */
#  define PLU_DEBUG(s) printf(s)
   /* basic debug output: sprintf, one parameter */
#  define PLU_DEBUG_1(s, par1) printf(s, par1)
   /* basic debug output: sprintf, two parameters */
#  define PLU_DEBUG_2(s, par1, par2) printf(s, par1, par2)
   /* The following four macros implement a set of assertions
    * to make sure that the Lua stack is balanced between
    * entering a chunk of code and exiting it. The
    * PLU_LEAVE_STACKASSERT_MODIFIED macro allows for specifying
    * an expected change in stack usage. */
   /* Usage:
    * PLU_dSTACKASSERT;
    * ... get a lua_State *L ...
    * PLU_ENTER_STACKASSERT(L);
    * ... code that does things to the Lua stack here
    * PLU_LEAVE_STACKASSERT(L);
    *
    * PLU_LEAVE_STACKASSERT will throw a Perl exception if
    * the stack isn't at the same index as at the start.
    */
#  define PLU_dSTACKASSERT  \
    int stackassertelems;   \
    char * stackassertfile; \
    int stackassertline

#  define PLU_ENTER_STACKASSERT(L)      \
  STMT_START {                          \
    stackassertelems = lua_gettop((L)); \
    stackassertfile = __FILE__;         \
    stackassertline = __LINE__;         \
  } STMT_END

#  define PLU_LEAVE_STACKASSERT(L) \
    PLU_LEAVE_STACKASSERT_MODIFIED(L, 0)

#  define PLU_LEAVE_STACKASSERT_MODIFIED(L, change)                           \
  STMT_START {                                                                \
    if (lua_gettop((L)) != stackassertelems + (change)) {                     \
      croak("Lua stack is unbalanced. "                                       \
            "At start: %i elements (at %s:%i). Now: %i elements "             \
            "(at %s:%i)", stackassertelems, stackassertfile, stackassertline, \
            lua_gettop((L)), __FILE__, __LINE__);                             \
    }                                                                         \
  } STMT_END

  /* Print Lua stack size to STDERR */
#  define PLU_DEBUG_STACKSIZE(L, id) \
    warn("Lua stack size (%s): %i\n", id, (int)lua_gettop(L));
#endif

#endif
