#include "plu_parse_kw.h"
#include "plu_debug.h"

#include "plu_global_state.h"
#include "plu_op.h"
#include "plu_lua.h"

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
  SV *lua_code;
  unsigned int ndelimchars = 1;
  char *code_str;
  STRLEN code_len;
  SV *lua_func_name_sv;
  int status;

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

  lua_func_name_sv = newSVpvf("_prl%lu", (unsigned long)PLU_global_lua_func_count++);
  sv_2mortal(lua_func_name_sv); /* auto-cleanup on exception */
  lua_code = newSVpvf("function %s()\n", SvPVX(lua_func_name_sv));
  sv_2mortal(lua_code); /* auto-cleanup on exception */
  sv_catpvn(lua_code, code_str, (STRLEN)code_len);
  sv_catpvs(lua_code, "\nend\n");

  code_str = SvPV(lua_code, code_len);

  plu_compile_lua_block_or_croak(aTHX_ code_str, code_len);
  /*printf("'%s'\n", lua_typename(PLU_lua_int, lua_type(PLU_lua_int, -1)));*/
  /*sv_dump(lua_code);*/

  /* FIXME just taking the pointer to a Lua function off of the stack doesn't cut
   * it as there seems to be no way to put it back and execute it :( */
  /*lua_fun = (void *)lua_topointer(PLU_lua_int, -1);
  lua_pop(PLU_lua_int, 1);
  */

  /*status = lua_pcall(PLU_lua_int, 0, LUA_MULTRET, 0);*/
  status = lua_pcall(PLU_lua_int, 0, 0, 0);
  if (status != 0)
    croak("Failed to run script: %s\n", lua_tostring(PLU_lua_int, -1));

  /* FIXME just playing... */
  *op_ptr = plu_prepare_custom_op(aTHX_ SvPVX(lua_func_name_sv));

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

