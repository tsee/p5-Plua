/* for performance */
#define PERL_NO_GET_CONTEXT

/* For versions of ExtUtils::ParseXS > 3.04_02, we need to
 * explicitly enforce exporting of XSUBs since we want to
 * refer to them using XS(). This isn't strictly necessary,
 * but it's by far the simplest way to be backwards-compatible.
 * This is for Lua functions.
 */
#define PERL_EUPXS_ALWAYS_EXPORT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include <string.h>
#include <assert.h>

#include "plu_debug.h"
#include "plu_inline.h"
#include "plu_global_state.h"
#include "plu_table.h"
#include "plu_lua_function.h"
#include "plu_lua.h"

MODULE = PLua PACKAGE = PLua
PROTOTYPES: DISABLE

BOOT:
  plu_init_global_state(aTHX);

INCLUDE: xs/Function.xs

INCLUDE: xs/Table.xs
