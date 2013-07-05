#include "pz_op.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pz_debug.h"
#include "pz_global_state.h"

OP *
pz_pp_custom(pTHX)
{
  dVAR; dSP;
  pz_op_aux_t *aux;
  aux = (pz_op_aux_t *)PL_op->op_targ;

  if (aux->test != NOT_IN_PAD) {
    SV *s = PAD_SVl(aux->test);
    sv_setiv_mg(s, SvIV(s)+1);
  }

  PZ_DEBUG("Finished executing OP.\n");
  RETURN;
}


/* Hook that will free the OP aux structure of our custom ops */
/* FIXME this doesn't appear to actually be called for all ops -
 *       specifically NOT for our custom OP. Is this because the
 *       custom OP isn't wired up correctly? */
void
pz_op_free_hook(pTHX_ OP *o)
{
  if (PZ_orig_opfreehook != NULL)
    PZ_orig_opfreehook(aTHX_ o);

  /* printf("cleaning %s\n", OP_NAME(o)); */
  if (o->op_ppaddr == pz_pp_custom) {
    PZ_DEBUG("Cleaning up custom OP's pz_op_aux_t\n");
    pz_op_aux_t *aux = (pz_op_aux_t *)o->op_targ;
    free(aux);
    o->op_targ = 0; /* important or Perl will use it to access the pad */
  }
}


OP *
pz_prepare_custom_op(pTHX)
{
  OP *op;
  pz_op_aux_t *aux;

  NewOp(1101, op, 1, OP);
  op->op_type = (OPCODE)OP_CUSTOM;
  op->op_next = (OP *)op;
  op->op_private = 0;
  op->op_flags = 0;

  /* Set it's implementation ptr */
  op->op_ppaddr = pz_pp_custom;

  /* Init aux struct */
  aux = malloc(sizeof(pz_op_aux_t));
  /* aux->jit_fun = NULL; */
  aux->saved_op_targ = 0;
  /* aux->saved_op_targ = origop->op_targ; */ /* save in case needed for sassign optimization */

  /* It may turn out that op_targ is not safe to use for custom OPs because
   * some core functions may meddle with it. But chances are it's fine.
   * If not, we'll need to become extra-creative... */
  op->op_targ = (PADOFFSET)PTR2UV(aux);

  return op;
}
