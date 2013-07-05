#ifndef PZ_OP_H_
#define PZ_OP_H_

/* Code related to the run-time implementation of the actual
 * custom OP. */

#include <EXTERN.h>
#include <perl.h>

/* The struct of pertinent per-OP instance
 * data that we attach to each OP. */
typedef struct {
  /* void (*fun)(void); */
  PADOFFSET test;
  void *lua_fun;
  PADOFFSET saved_op_targ; /* Replacement for custom OP's op_targ if necessary */
} pz_op_aux_t;

/* The generic custom OP implementation - push/pop function */
OP *pz_pp_custom(pTHX);

/* Hook that will free the OP aux structure of our custom ops */
void pz_op_free_hook(pTHX_ OP *o);

/* Set up OP without doing actual compilation. */
OP *pz_prepare_custom_op(pTHX, void *lua_func);

#endif
