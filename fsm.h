/*
 * fsm.h
 *
 *  Created on: 1 de mar. de 2016
 *      Author: Administrador
 */

#ifndef FSM_H_
#define FSM_H_

typedef struct fsm_t fsm_t;

//FSM output and input functions
typedef unsigned char (*fsm_input_func_t) (fsm_t*);
typedef void (*fsm_output_func_t) (fsm_t*);

//FSM transitions table
typedef struct fsm_trans_t {
  int orig_state;
  fsm_input_func_t in;
  int dest_state;
  fsm_output_func_t out;
} fsm_trans_t;

//FSM object
struct fsm_t {
  int current_state;
  fsm_trans_t* tt;
  void* user_data;
};

void fsm_init (fsm_t* this, int state, fsm_trans_t* tt, void* user_data);
void fsm_fire (fsm_t* this);

#endif /* FSM_H_ */
