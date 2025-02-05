#ifndef CMD_EXEC_H
#define  CMD_EXEC_H

#include "token.h"

typedef enum cmd_type_t{
    C_NONE,
    C_IF,
    C_THEN,
    C_ELSE,
    C_FI,
    C_EXIT,
    C_ASSIGN,
    C_ENV,
    C_SET,
    C_UNSET,
    C_EXPORT
} cmd_type_t;

enum if_state_t{
    IS_NONE = 0,                //out of 'if' block
    IS_WAIT_COND,
    IS_WAIT_THEN,               //wait condition expression
    IS_THEN_BLOCK,              //expect then block and execute it till 'else' or 'fi'
    IS_ELSE_BLOCK               //expect then block and skip it till 'else' or 'fi' if 'else' found exec till 'fi'
};

enum if_stat_result_t{
    ISR_NONE = 0,
    ISR_FAILURE,
    ISR_SUCCESS
};

//parse user input and execute commands
void process_shell_cmds(char *args);

//get user input and return it in buffer
char* get_shell_cmd();
#endif