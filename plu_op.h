#ifndef PLU_OP_H_
#define PLU_OP_H_

/* Code related to the run-time implementation of the actual
 * custom OP. */

#include <EXTERN.h>
#include <perl.h>

/* The struct of pertinent per-OP instance
 * data that we attach to each OP. */
typedef struct {
  int func_registry_idx;
  PADOFFSET saved_op_targ; /* Replacement for custom OP's op_targ if necessary */
} plu_op_aux_t;

/* The generic custom OP implementation - push/pop function */
OP *plu_pp_custom(pTHX);

/* Hook that will free the OP aux structure of our custom ops */
void plu_op_free_hook(pTHX_ OP *o);

/* Set up OP without doing actual compilation. */
OP *plu_prepare_custom_op(pTHX_ const int lua_func_registry_idx);

/* Generating NULL OPs for things without run-time impact */
OP *plu_prepare_null_op(pTHX);

#endif
