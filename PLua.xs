#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include <assert.h>

#include "plu_debug.h"
#include "plu_inline.h"
#include "plu_global_state.h"

MODULE = PLua PACKAGE = PLua
PROTOTYPES: ENABLE

BOOT:
  plu_init_global_state(aTHX);

