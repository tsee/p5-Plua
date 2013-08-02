#include "plu_lua_syntax_ext.h"
#include "plu_debug.h"

#include "plu_global_state.h"
#include "plu_op.h"
#include "plu_lua.h"

void
plu_implement_lua_lexicals(pTHX_ SV *lcode)
{
  /* This just delegates to Perl functions - regexes
   * are really rather powerful and calling Perl regexes from
   * C is like making mince with a colander and a plunger. */
  SV *lexical_rv;
  HV *lexical_hv;
  AV *lexical_ary;
  HE *he;
  unsigned int i, n;
  PADOFFSET padoff;
  SV *name;
  char *str;
  STRLEN len;

  /* First, scan the code for $foo.int and frients */

  {
    dSP;
    int count;
    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    XPUSHs(lcode);
    PUTBACK;

    count = call_pv("PLua::_scan_lua_code", G_SCALAR);

    SPAGAIN;

    if (count != 1)
      croak("Panic: Scalar context and not exactly one return value? Madness!");

    /* These lexicals will need looking up with pad_findmy */
    lexical_rv = POPs;
    lexical_hv = (HV *)SvRV(lexical_rv);

    SvREFCNT_inc(lexical_rv);
    PUTBACK;
    FREETMPS;
    LEAVE;
    sv_2mortal(lexical_rv);
  }

  /* Now do all PAD lookups for lexicals in the HV */
  /* Use a tmp array to avoid modifying a hash being looped over.
   * This is obvioiusly possible by fuzzing with the HE, but
   * my perl API fu is weak and I just want it working for now. FIXME */
  lexical_ary = newAV();
  sv_2mortal((SV *)lexical_ary);
  (void)hv_iterinit(lexical_hv);
  while ((he = hv_iternext(lexical_hv))) {
    SV *k = hv_iterkeysv(he);
    SvREFCNT_inc(k);
    av_push(lexical_ary, k);
  }

  n = (unsigned int)av_len(lexical_ary)+1;
  for (i = 0; i < n; ++i) {
    name = *av_fetch(lexical_ary, i, 0);
    str = SvPV(name, len);
    padoff = pad_findmy(str, len, 0);
    if (LIKELY( padoff != NOT_IN_PAD ))
      (void)hv_store(lexical_hv, str, len, newSViv(padoff), 0);
    /* No else needed - skipping from HV will cause exception in
     * Perl code called further down. */
  }

  /* Now actually munge the code based on the PAD lookups. */
  {
    dSP;
    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    XPUSHs(lcode);
    XPUSHs(lexical_rv);
    PUTBACK;

    call_pv("PLua::_munge_lua_code", G_DISCARD);

    FREETMPS;
    LEAVE;
  }
}

