#include "plu_parse_kw.h"
#include "plu_debug.h"

#include "plu_global_state.h"
#include "plu_op.h"
#include "plu_lua.h"
#include "plu_lua_syntax_ext.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


/* Append single character to string SV, possibly upgrading
 * it to UTF8. From Function::Parameters */
static void
S_sv_cat_c(pTHX_ SV *sv, U32 c) {
  char ds[UTF8_MAXBYTES + 1], *d;
  d = (char *)uvchr_to_utf8((U8 *)ds, c);
  if (d - ds > 1) {
    sv_utf8_upgrade(sv);
  }
  sv_catpvn(sv, ds, d - ds);
}


#define MY_UNI_IDFIRST(C) isIDFIRST_uni(C)
#define MY_UNI_IDCONT(C)  isALNUM_uni(C)

/* Scan one identifier. From Function::Parameters */
static SV *
S_scan_ident(pTHX)
{
  bool at_substart;
  I32 c;
  SV *sv = sv_2mortal(newSVpvs(""));
  if (lex_bufutf8())
    SvUTF8_on(sv);

  at_substart = TRUE;
  c = lex_peek_unichar(0);

  while (c != -1) {
    if (at_substart ? MY_UNI_IDFIRST(c) : MY_UNI_IDCONT(c)) {
      lex_read_unichar(0);
      S_sv_cat_c(aTHX_ sv, c);
      at_substart = FALSE;
      c = lex_peek_unichar(0);
    }
    else {
      break;
    }
  }

  return SvCUR(sv) ? sv : NULL;
}


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
 * from right after the lua keyword. Croaks on error */
static SV *
S_parse_lua_block(pTHX)
{
  unsigned int ndelimchars;
  char *code_str;
  STRLEN code_len;
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
  return lua_code_sv;
}

/* Compiles an embedded lua code block ("lua {{{ ... }}}") to
 * a Perl custom OP. Errors handled as Perl exceptions. */
void
S_compile_embedded_lua_block(pTHX_ OP **op_ptr)
{
  SV *lua_code_sv;
  int lua_reg_idx;
  char *code_str;
  STRLEN code_len;

  /* This handles errors with exceptions: */
  lua_code_sv = S_parse_lua_block(aTHX);

  /* Munge code to support our shady Perl-like
   * syntax for lexical access */
  plu_implement_lua_lexicals(aTHX_ lua_code_sv);
  code_str = SvPV(lua_code_sv, code_len);

  /* Actually do the code => Lua function compilation */
  plu_compile_lua_block_or_croak(aTHX_ code_str, code_len);

  /* Get registry index for the just-compiled function */
  lua_reg_idx = luaL_ref(PLU_lua_int, LUA_REGISTRYINDEX);
  *op_ptr = plu_prepare_custom_op(aTHX_ lua_reg_idx);
}

static void
S_skip_lua_comments(pTHX)
{
  /* TODO implement */

  /* Remember to skip space after the comments */
  lex_read_space(0);
}

/* Parses Lua function parameters, starting and ending with parenthesis
 * and returns them as a string. Croaks on error. */
static SV *
S_parse_lua_function_parameters(pTHX)
{
  SV *sv;
  I32 c;
  int done = 0;

  enum State {
    IDENTIFIER,
    SEPARATOR
  };

  sv = sv_2mortal(newSVpvs(""));
  if (lex_bufutf8())
    SvUTF8_on(sv);

  lex_read_space(0);

  c = lex_read_unichar(0);
  if (c != '(')
    croak("Syntax error: Expected start of function parameter list '('");
  S_sv_cat_c(aTHX_ sv, c);

  /* Special case: () */
  lex_read_space(0);
  c = lex_peek_unichar(0);
  if (c == ')') {
    lex_read_unichar(0);
    sv_catpvs(sv, ")");
    return sv;
  }

  enum State state = IDENTIFIER;

  c = 0;
  while (c != -1) {
    lex_read_space(0);
    S_skip_lua_comments(aTHX);

    if (state == IDENTIFIER) {
      SV *ident = S_scan_ident(aTHX);
      if (ident != NULL)
        sv_catsv_nomg(sv, ident);
      else { /* attempt to scan '...' instead, must be followed by end-of-list */
        
        unsigned int i;
        for (i = 0; i < 3; ++i) {
          c = lex_read_unichar(0);
          if (c != '.')
            croak("Syntax error: While parsing Lua function parameters, "
                  "expected identifier or '...', got '%c'\n", c);
        }
        sv_catpvs(sv, "...");

        /* Now ensure that '...' was at end of list */
        lex_read_space(0);
        S_skip_lua_comments(aTHX);
        c = lex_read_unichar(0);
        if (c != ')')
          croak("Syntax error: While parsing Lua function parameters, "
                "expected end of list after '...', got '%c'\n", c);
        sv_catpvs(sv, ")");
        /* Just in case */
        lex_read_space(0);
        S_skip_lua_comments(aTHX);
        done = 1;
        break;
      } /* end 'scan ...' */
      state = SEPARATOR;
    } /* end if state == IDENTIFIER */
    else { /* state == SEPARATOR */
      c = lex_read_unichar(0);
      if (c == ')') { /* DONE */
        sv_catpvs(sv, ")");
        /* Just in case */
        lex_read_space(0);
        S_skip_lua_comments(aTHX);
        done = 1;
        break;
      }
      else if (c == ',') {
        sv_catpvs(sv, ",");
      }
      else {
        croak("Syntax error: Expected separator ',' or "
              "end of parameter list, got '%c' instead", c);
      }
      state = IDENTIFIER;
    }
  }

  /* In case we just prematurely hit c == -1 */
  if (done == 0)
    croak("Syntax error: Reached end of program before the end of the Lua "
          "parameter list");

  return sv;
}

/* Compiles an embedded lua function ("lua_function (a,b,...) {{{ ... }}}") to
 * a Perl custom OP. Errors handled as Perl exceptions. */
void
S_compile_embedded_lua_function(pTHX_ OP **op_ptr)
{
  SV *lua_code_sv;
  int lua_reg_idx;
  char *code_str;
  STRLEN code_len;
  SV *lua_func_params;
  SV *func_name;
  SV *full_func_code;
  SV *perl_coderef;

  lex_read_space(0);
  func_name = S_scan_ident(aTHX);
  if (!func_name)
    croak("Syntax error: Expected Lua function name");

  /* This handles errors with exceptions: */
  lex_read_space(0);
  lua_func_params = S_parse_lua_function_parameters(aTHX);
  /*printf("PARAMS: '%s'\n", SvPV_nolen(lua_func_params));*/

  /* This handles errors with exceptions: */
  lex_read_space(0);
  lua_code_sv = S_parse_lua_block(aTHX);
  /*printf("CODE: '%s'\n", SvPV_nolen(lua_code_sv)); */
  /*printf("'%s'\n", PL_parser->bufptr);*/

  /* Munge code to support our shady Perl-like
   * syntax for lexical access, modifying lua_code_sv */
  full_func_code
    = plu_implement_embedded_lua_function(aTHX_
                                          func_name,
                                          lua_func_params,
                                          lua_code_sv);
  code_str = SvPV(full_func_code, code_len);

  /* Actually do the code => Lua function compilation */
  plu_compile_lua_block_or_croak(aTHX_ code_str, code_len);

  perl_coderef = plu_new_function_object_perl(aTHX_ PLU_lua_int);
  /* FIXME install named sub here */

  *op_ptr = plu_prepare_null_op(aTHX);
}

/* Main keyword plugin hook */
int
plu_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr) {
  int ret;

  if (keyword_len == 3 && memcmp(keyword_ptr, "lua", 3) == 0)
  {
    SAVETMPS;
    S_compile_embedded_lua_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  }
  else if ( keyword_len == 12
            && memcmp(keyword_ptr, "lua_function", 12) == 0 )
  {
    SAVETMPS;
    S_compile_embedded_lua_function(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  }
  else {
    ret = (*PLU_next_keyword_plugin)(aTHX_ keyword_ptr, keyword_len, op_ptr);
  }

  return ret;
}

