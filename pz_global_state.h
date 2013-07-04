#ifndef PZ_GLOBAL_STATE_H_
#define PZ_GLOBAL_STATE_H_

/* The less there is in here, the better. */

#include <EXTERN.h>
#include <perl.h>

/* The custom op definition structure */
extern XOP PZ_xop;

/* For chaining the actual keyword plugin */
extern int (*PZ_next_keyword_plugin)(pTHX_ char *, STRLEN, OP **);

/* Original opfreehook - we wrap this to free JIT OP aux structs */
extern Perl_ophook_t PZ_orig_opfreehook;

/* Initialize global state like custom op description, etc. */
void pz_init_global_state(pTHX);

/* End-of-global-destruction cleanup hook. */
/* void pz_global_state_final_cleanup(pTHX_ void *ptr); */


#endif
