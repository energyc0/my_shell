#ifndef IF_STACK_H
#define IF_STACK_H

#include "cmd_exec.h"

struct shell_state{
    enum if_stat_result_t if_res;
    enum if_state_t if_state;
};


enum if_state_t get_current_state();
enum if_stat_result_t get_current_result();

void change_current_state(enum if_state_t st);
void change_current_result(enum if_stat_result_t res);

void push_if_statement(struct shell_state* st);
void pop_if_statement();

void clear_if_stack();
#endif