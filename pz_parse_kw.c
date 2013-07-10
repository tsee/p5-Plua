#include "pz_parse_kw.h"
#include "pz_debug.h"

#include "pz_global_state.h"
#include "pz_op.h"
#include "pz_lua.h"

static void
free_op(pTHX_ void *ptr) {
  OP **opp = ptr;
  op_free(*opp);
  Safefree(opp);
}

static void
my_sv_cat_c(pTHX_ SV *sv, U32 c) {
  U8 ds[UTF8_MAXBYTES + 1], *d;
  d = uvchr_to_utf8(ds, c);
  if (d - ds > 1) {
    sv_utf8_upgrade(sv);
  }
  sv_catpvn(sv, (char *)ds, d - ds);
}

#define MY_UNI_IDFIRST(C) isIDFIRST_uni(C)
#define MY_UNI_IDCONT(C)  isALNUM_uni(C)

static SV *
my_scan_word(pTHX) {
  I32 c;
  SV *sv;

  c = lex_peek_unichar(0);
  if (c == -1 || !MY_UNI_IDFIRST(c)) {
    return NULL;
  }
  lex_read_unichar(0);

  sv = sv_2mortal(newSVpvs(""));
  if (lex_bufutf8()) {
    SvUTF8_on(sv);
  }

  my_sv_cat_c(aTHX_ sv, c);

  while ((c = lex_peek_unichar(0)) != -1 && MY_UNI_IDCONT(c)) {
    lex_read_unichar(0);
    my_sv_cat_c(aTHX_ sv, c);
  }

  return sv;
}

static void
scan_lua_block_delim(pTHX_ const unsigned int ndelimchars, char **outstring, STRLEN *outstringlen)
{
  while (1) {
    char* const end = PL_parser->bufend;
    char *s = PL_parser->bufptr;
    unsigned int ndelim = 0;
    while (end-s > ndelimchars) {
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
parse_lua_block(pTHX_ OP **op_ptr)
{
  int save_ix;
  I32 c;
  SV *delim;
  SV *lua_code;
  PADOFFSET test_padofs;
  unsigned int ndelimchars = 1;
  char *code_str;
  STRLEN code_len;
  int status;
  void *lua_fun;

  lex_read_space(0);

  /* Let's use one or multiple opening curlies as delimiter ... */
  c = lex_read_unichar(0);
  if (c != '{')
    croak("Can't parse Lua block after 'lua'");

  while (1) {
    c = lex_read_unichar(0);
    if (c != '{')
      break;
    ndelimchars++;
  }

  lex_read_space(0);

  lex_read_space(0);

  /*
  Newx(return_op, 1, OP *);
  *return_op = NULL;
  SAVEDESTRUCTOR_X(free_op, return_op);
  */

  scan_lua_block_delim(aTHX_ ndelimchars, &code_str, &code_len);
  if (code_str == NULL)
    croak("Syntax error: cannot find Lua block delimiter");

  lua_code = newSVpvf(
    "function _prl%lu()\n",
    (unsigned long)PZ_global_lua_func_count++
  );
  sv_2mortal(lua_code); /* auto-cleanup on exception */
  sv_catpvn(lua_code, code_str, (STRLEN)code_len);
  sv_catpvs(lua_code, "\nend\n");

  sv_dump(lua_code);
  code_str = SvPV(lua_code, code_len);

  pz_compile_lua_block_or_croak(aTHX_ code_str, code_len);
  /*printf("'%s'\n", lua_typename(PZ_lua_int, lua_type(PZ_lua_int, -1)));*/
  /*sv_dump(lua_code);*/

  /* FIXME just taking the pointer to a Lua function off of the stack doesn't cut
   * it as there seems to be no way to put it back and execute it :( */
  /*lua_fun = (void *)lua_topointer(PZ_lua_int, -1);
  lua_pop(PZ_lua_int, 1);
  */

  /*status = lua_pcall(PZ_lua_int, 0, LUA_MULTRET, 0);*/
  status = lua_pcall(PZ_lua_int, 0, 0, 0);
  if (status != 0) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(PZ_lua_int, -1));
  }

  /* FIXME just playing... */
  *op_ptr = pz_prepare_custom_op(aTHX_ lua_fun);

  test_padofs = pad_findmy("$foo", 4, 0);
  ((pz_op_aux_t *)(*op_ptr)->op_targ)->test = test_padofs;
}
 


int
pz_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr) {
  int ret;

  if (keyword_len == 3 && memcmp(keyword_ptr, "lua", 3) == 0) {
    SAVETMPS;
    parse_lua_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  } else {
    ret = (*PZ_next_keyword_plugin)(aTHX_ keyword_ptr, keyword_len, op_ptr);
  }

  return ret;
}

