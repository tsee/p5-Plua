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
scan_heredoc_delim(pTHX_ char *delim, STRLEN delimlen)
{
  while(1) {
    char *end = PL_parser->bufend;
    char *s = PL_parser->bufptr;
    while (s != end) {
      if (*s == '\n') {
        if (end-s < delimlen+1) {
          break;
        }
        else if (strncmp(s+1, delim, (size_t)delimlen) == 0) {
          /* FIXME scan whitespace to end of line here */
          SV *rv;
          rv = sv_2mortal(newSVpvn(PL_parser->bufptr, s - PL_parser->bufptr));
          lex_read_to(s+delimlen+1);
          return rv;
        }
        else {
          s += delimlen;
        }
      }
      else {
        s++;
      }
    }
    if ( !lex_next_chunk(LEX_KEEP_PREVIOUS) )
      croak("syntax error");
  }
  return NULL;
}

static void
parse_zoom_block(pTHX_ OP **op_ptr)
{
  int save_ix;
  I32 c;
  SV *delim;
  SV *heredoc;
  PADOFFSET test_padofs;

  lex_read_space(0);

  c = lex_read_unichar(0);
  if (c != '<')
    croak("Can't parse HERE-doc after 'zoom'");

  c = lex_read_unichar(0);
  if (c != '<')
    croak("Can't parse HERE-doc after 'zoom'");

  lex_read_space(0);
  delim = my_scan_word(aTHX);

  lex_read_space(0);
  c = lex_read_unichar(0);
  if (c != ';')
    croak("Can't parse HERE-doc after 'zoom'");

  /*
  Newx(return_op, 1, OP *);
  *return_op = NULL;
  SAVEDESTRUCTOR_X(free_op, return_op);
  */

  heredoc = scan_heredoc_delim(aTHX_ SvPVX(delim), SvCUR(delim));
  /*sv_dump(heredoc);*/

  /* FIXME just playing... */
  *op_ptr = pz_prepare_custom_op(aTHX);

  test_padofs = pad_findmy("$foo", 4, 0);
  ((pz_op_aux_t *)(*op_ptr)->op_targ)->test = test_padofs;
}
 


int
pz_my_keyword_plugin(pTHX_ char *keyword_ptr, STRLEN keyword_len, OP **op_ptr) {
  int ret;

  if (keyword_len == 4 && memcmp(keyword_ptr, "zoom", 4) == 0) {
    SAVETMPS;
    parse_zoom_block(aTHX_ op_ptr);
    ret = KEYWORD_PLUGIN_STMT;
    FREETMPS;
  } else {
    ret = (*PZ_next_keyword_plugin)(aTHX_ keyword_ptr, keyword_len, op_ptr);
  }

  return ret;
}

