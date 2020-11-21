/*
 * fsm.c
 *
 *  Created on: 1 de mar. de 2016
 *      Author: Administrador
 */

#include "fsm.h"

void
fsm_init (fsm_t* this, int state, fsm_trans_t* tt, void* user_data)
{
  this->current_state = state;
  this->tt = tt;
  this->user_data = user_data;
}


void
fsm_fire (fsm_t* this)
{
  fsm_trans_t* t;
  for (t = this->tt; t->orig_state >= 0; ++t) {
    if ((this->current_state == t->orig_state) && t->in(this)) {
      this->current_state = t->dest_state;
      if (t->out)
        t->out(this);
      break;
    }
  }
}

