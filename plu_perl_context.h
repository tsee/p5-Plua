#ifndef PLU_PERL_CONTEXT_H_
#define PLU_PERL_CONTEXT_H_

/* Macros that hide any PERL_IMPLICIT_CONTEXT switcharoo regarding THX or non-THX
 * enabled perls from the user.
 * PLU_dTHX just compiles to an empty Perl THX declaration.
 * PLU_N_THX_ARGS indicates the number of closure arguments on the
 *   Lua stack to include for the Perl THX (0 or 1). See lua_pushcclosure.
 * PLU_PUSH_THX pushes the actual THX on the Lua stack - or doesn't.
 * PLU_GET_THX fetches and sets the THX from the Lua upvalues.
 *
 * This assumes that the Perl THX is the first upvalue of the C closure.
 * That means you always have to add PLU_N_THX_ARGS to your enumrated
 * upvalues if you use any others.
 */

#ifdef PERL_IMPLICIT_CONTEXT
# ifndef PLU_PUSH_THX
#   define PLU_PUSH_THX(L) lua_pushlightuserdata(L, (void *)aTHX)
#   define PLU_N_THX_ARGS 1
#   define PLU_dTHX pTHX
#   define PLU_GET_THX(L) aTHX = (tTHX)lua_touserdata(L, lua_upvalueindex(1))
# endif
#else
# ifndef PLU_PUSH_THX
#   define PLU_PUSH_THX(L) NOOP
#   define PLU_N_THX_ARGS 0
#   define PLU_dTHX dNOOP
#   define PLU_GET_THX(L) NOOP
# endif
#endif

#endif
