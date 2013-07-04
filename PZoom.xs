#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include <assert.h>

#include "pz_debug.h"
#include "pz_inline.h"
#include "pz_global_state.h"

MODULE = PZoom PACKAGE = PZoom
PROTOTYPES: ENABLE

BOOT:
  pz_init_global_state(aTHX);

