#include "pz_parse_kw.h"
#include "pz_debug.h"

#include "pz_global_state.h"
#include "pz_op.h"

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

static SV *
scan_lua_block_delim(pTHX_ const unsigned int ndelimchars)
{
  while(1) {
    char* const end = PL_parser->bufend;
    char *s = PL_parser->bufptr;
    unsigned int ndelim = 0;
    while (end-s > ndelimchars) {
      if (*s == '}') {
        ndelim++;
        if (ndelim == ndelimchars) {
          SV *rv;
          rv = sv_2mortal(newSVpvn(PL_parser->bufptr, s - PL_parser->bufptr - ndelimchars+1));
          lex_read_to(s+1); /* skip Perl's lexer/parser ahead to end of Lua block */
          return rv;
        }
      }
      else {
        ndelim = 0;
      }
      s++;
    }
    if ( !lex_next_chunk(LEX_KEEP_PREVIOUS) )
      croak("Syntax error: cannot find Lua block delimiter");
  }
  return NULL;
}

static int
compile_lua_block(pTHX_ char *code, STRLEN len)
{
  int status;
  status = luaL_loadbuffer(PZ_lua_int, code, len, "_INLINED_LUA");
  return status;
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
  int compile_status;
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

  lua_code = scan_lua_block_delim(aTHX_ ndelimchars);
  code_str = SvPV(lua_code, code_len);
  compile_status = compile_lua_block(aTHX_ code_str, code_len);
  switch (compile_status) {
  case 0:
    break;
  case LUA_ERRSYNTAX:
  default:
    croak("Couldn't compile inline code");
  }
  /*printf("'%s'\n", lua_typename(PZ_lua_int, lua_type(PZ_lua_int, -1)));*/
  /*sv_dump(lua_code);*/

  lua_fun = lua_topointer(PZ_lua_int, -1);
  lua_pop(PZ_lua_int, 1);
  /*compile_status = lua_pcall(PZ_lua_int, 0, LUA_MULTRET, 0);*/
  /*
  compile_status = lua_pcall(PZ_lua_int, 0, 0, 0);
  if (compile_status != 0) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(PZ_lua_int, -1));
  }
  */

  /* FIXME just playing... */
  *op_ptr = pz_prepare_custom_op(aTHX, lua_fun);

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

