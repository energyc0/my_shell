#ifndef IF_STATE_H
#define IF_STATE_H

#include "cmd_exec.h"

enum if_state_t{
    IS_NONE,
    IS_IF_COND,
    IS_THEN_BLOCK,
    IS_ELSE_BLOCK
};

//call on setup
void setup_if_state_buffer();

//return current if_state
enum if_state_t get_if_state();

//changing if_state without checking errors
void change_if_state(enum if_state_t st);

//return 1 if cmd is in 'if' condition block, 'then' or 'else' block, 0 otherwise
int preserve_cmd(cmd_t cmd, cmd_type_t t);

//set the state out of 'if' statement and reset command buffers
void reset_if_state();
#endif