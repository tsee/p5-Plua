#include "plu_parse_kw.h"
#include "plu_debug.h"

#include "plu_global_state.h"
#include "plu_op.h"
#include "plu_lua.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static void
S_scan_lua_block_delim(pTHX_ const unsigned int ndelimchars, char **outstring, STRLEN *outstringlen)
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
          *outstringlen = (STRLEN)(s - PL_parser->bufptr - ndelimchars);
          lex_read_to(s+1); /* skip Perl's lexer/parser ahead to end of Lua block */
          return;
        }
      }
      else
        ndelim = 0;
      s++;
    }
    if ( !lex_next_chunk(LEX_KEEP_PREVIOUS) )
      croak("Syntax error: cannot find Lua block delimiter");
  }
}

static void
S_parse_lua_block(pTHX_ OP **op_ptr)
{
  I32 c;
  unsigned int ndelimchars = 1;
  char *code_str;
  STRLEN code_len;
  int lua_reg_idx;

  lex_read_space(0);

  /* Let's use one or multiple opening curlies as delimiter ... */
  c = lex_read_unichar(0);
  if (c != '{')
    croak("Can't parse Lua block after 'lua'");

  /* FIXME do we need to back up one char at the end of this? */
  while (1) {
    c = lex_read_unichar(0);
    if (c != '{')
      break;
    ndelimchars++;
  }

  lex_read_space(0);

  lex_read_space(0);

  S_scan_lua_block_delim(aTHX_ ndelimchars, &code_str, &code_len);
  if (code_str == NULL)
    croak("Syntax error: cannot find Lua block delimiter");

  plu_compile_lua_block_or_croak(aTHX_ code_str, code_len);
  lua_reg_idx = luaL_ref(PLU_lua_int, LUA_REGISTRYINDEX);

  *op_ptr = plu_prepare_custom_op(aTHX_ lua_reg_idx);

  /*test_padofs = pad_findmy("$foo", 4, 0);
  ((plu_op_aux_t *)(*op_ptr)->op_targ)->test = test_padofs;*/
}
 


int
plu_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr) {
  int ret;

  if (keyword_len == 3 && memcmp(keyword_ptr, "lua", 3) == 0) {
    SAVETMPS;
    S_parse_lua_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  } else {
    ret = (*PLU_next_keyword_plugin)(aTHX_ keyword_ptr, keyword_len, op_ptr);
  }

  return ret;
}

