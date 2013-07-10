#include "plu_op.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "plu_debug.h"
#include "plu_global_state.h"
#include "plu_lua.h"

OP *
plu_pp_custom(pTHX)
{
  dVAR; dSP;
  plu_op_aux_t *aux;
  int status;

  aux = (plu_op_aux_t *)PL_op->op_targ;

  status = plu_call_lua_func(aTHX_ aux->lua_func_name);
  switch (status) {
    case 0:
      break;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    case LUA_ERRERR:
      {
        SV *err = plu_get_lua_errmsg(aTHX);
        lua_pop(PLU_lua_int, 1);
        croak("%s", SvPVX(err));
        break;
      }
  }
  /*if (aux->test != NOT_IN_PAD) {
    SV *s = PAD_SVl(aux->test);
    sv_setiv_mg(s, SvIV(s)+1);
  }*/


  /*PLU_DEBUG("Finished executing OP.\n");*/
  RETURN;
}


/* Hook that will free the OP aux structure of our custom ops */
/* FIXME this doesn't appear to actually be called for all ops -
 *       specifically NOT for our custom OP. Is this because the
 *       custom OP isn't wired up correctly? */
void
plu_op_free_hook(pTHX_ OP *o)
{
  if (PLU_orig_opfreehook != NULL)
    PLU_orig_opfreehook(aTHX_ o);

  /* printf("cleaning %s\n", OP_NAME(o)); */
  if (o->op_ppaddr == plu_pp_custom) {
    PLU_DEBUG("Cleaning up custom OP's plu_op_aux_t\n");
    plu_op_aux_t *aux = (plu_op_aux_t *)o->op_targ;
    Safefree(aux->lua_func_name);
    Safefree(aux);
    o->op_targ = 0; /* important or Perl will use it to access the pad */
  }
}


OP *
plu_prepare_custom_op(pTHX, const char *lua_func_n)
{
  OP *op;
  plu_op_aux_t *aux;

  NewOp(1101, op, 1, OP);
  op->op_type = (OPCODE)OP_CUSTOM;
  op->op_next = (OP *)op;
  op->op_private = 0;
  op->op_flags = 0;

  /* Set it's implementation ptr */
  op->op_ppaddr = plu_pp_custom;

  /* Init aux struct */
  Newx(aux, 1, plu_op_aux_t);
  aux->lua_func_name = savepv(lua_func_n);
  aux->saved_op_targ = 0;
  /* aux->saved_op_targ = origop->op_targ; */ /* save in case needed for sassign optimization */

  /* It may turn out that op_targ is not safe to use for custom OPs because
   * some core functions may meddle with it. But chances are it's fine.
   * If not, we'll need to become extra-creative... */
  op->op_targ = (PADOFFSET)PTR2UV(aux);

  return op;
}
