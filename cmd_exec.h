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
    C_EXPORT,
    C_READ
} cmd_type_t;

enum if_state_t{
    IS_NONE = 0,                //out of 'if' block
    IS_WAIT_COND,               //wait condition expression
    IS_SUCCEEDED,                //wait for 'then', execute 'then' block
    IS_FAILED,                  //wait for 'then', execute 'else' block or nothing
    IS_DOING_THEN,              //doing 'then' block until 'else' or 'fi'
    IS_SKIPPING_THEN,           //skipping 'then' block until 'else' or 'fi'
    IS_DOING_ELSE,              //doing 'then' block until 'else' or 'fi'
    IS_SKIPPING_ELSE            //skipping 'then' block until 'else' or 'fi'    
};


void setup_child_sighandler();

//parse user input and execute commands
void process_shell_cmds(char *args);

//get user input and return it in buffer
char* get_shell_cmd();
#endif