#include "plu_parse_kw.h"
#include "plu_debug.h"

#include "plu_global_state.h"
#include "plu_op.h"
#include "plu_lua.h"
#include "plu_lua_syntax_ext.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Starting right after an opening set of curlies,
 * scan all the way to the matching closing set (ignoring that
 * there might be comments or string literals) and
 * return the string up to the closing curlies. */
static void
S_scan_lua_block_end(pTHX_ const unsigned int ndelimchars,
                           char **outstring,
                           STRLEN *outstringlen)
{
  while (1) {
    char* const end = PL_parser->bufend;
    char *s = PL_parser->bufptr;
    unsigned int ndelim = 0;
    while (end-s >= ndelimchars) {
      if (*s == '}') {
        ndelim++;
        if (ndelim == ndelimchars) {
          *outstring = PL_parser->bufptr;
          *outstringlen = (STRLEN)(s - PL_parser->bufptr - ndelimchars + 1);
          lex_read_to(s+1); /* skip Perl's lexer/parser ahead to end of Lua block */
          return;
        }
      }
      else
        ndelim = 0;
      s++;
    }
    if ( !lex_next_chunk(LEX_KEEP_PREVIOUS) ) {
      /* "Syntax error: cannot find Lua block delimiter" */
      *outstring = NULL;
      *outstringlen = 0;
      return;
    }
  }
}

/* Consume one or more {'s and return the count */
static unsigned int
S_parse_lua_block_delimiter(pTHX)
{
  unsigned int ndelimchars;
  I32 c;

  lex_read_space(0);

  /* Let's use one or multiple opening curlies as delimiter ... */
  c = lex_read_unichar(0);
  if (c != '{')
    return 0;
  ndelimchars = 1;

  /* Peek first to be able to not eat the first non-delimiter character */
  while (1) {
    c = lex_peek_unichar(0);
    if (c != '{')
      break;
    (void)lex_read_unichar(0);
    ndelimchars++;
  }

  return ndelimchars;
}

/* Consume a full block of Lua code including delimiters
 * from right after the lua keyword. Creates custom OP
 * in input parameter; croaks on error */
static void
S_parse_lua_block(pTHX_ OP **op_ptr)
{
  unsigned int ndelimchars;
  char *code_str;
  STRLEN code_len;
  int lua_reg_idx;
  SV *lua_code_sv;

  /* Count {'s */
  ndelimchars = S_parse_lua_block_delimiter(aTHX);
  if (ndelimchars == 0)
    croak("Syntax error: Can't find Lua block "
          "opening delimiter (one or multiple opening braces)");

  lex_read_space(0);

  /* Scan to end of matching } x ndelimchars */
  S_scan_lua_block_end(aTHX_ ndelimchars, &code_str, &code_len);

  if (code_str == NULL)
    croak("Syntax error: cannot find Lua "
          "block delimiter of %i closing braces", (int)ndelimchars);

  lua_code_sv = sv_2mortal(newSVpvn(code_str, code_len));
  plu_munge_lua_code(aTHX_ lua_code_sv);
  code_str = SvPV(lua_code_sv, code_len);

  plu_compile_lua_block_or_croak(aTHX_ code_str, code_len);

  /* Get registry index for the just-compiled function */
  lua_reg_idx = luaL_ref(PLU_lua_int, LUA_REGISTRYINDEX);
  *op_ptr = plu_prepare_custom_op(aTHX_ lua_reg_idx);
}
 


/* Main keyword plugin hook */
int
plu_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr) {
  int ret;

  if (keyword_len == 3 && memcmp(keyword_ptr, "lua", 3) == 0)
  {
    SAVETMPS;
    S_parse_lua_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  }
  /*else if ( keyword_len == 12
            && memcmp(keyword_ptr, "lua_function", 12) == 0 )
  {
    SAVETMPS;
    S_parse_lua_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  }*/
  else {
    ret = (*PLU_next_keyword_plugin)(aTHX_ keyword_ptr, keyword_len, op_ptr);
  }

  return ret;
}

