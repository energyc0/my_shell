#ifndef IF_STACK_H
#define IF_STACK_H

#include "cmd_exec.h"



enum if_state_t get_current_state();

void change_current_state(enum if_state_t st);

void push_if_statement(enum if_state_t st);
void pop_if_statement();

void clear_if_stack();
#endif