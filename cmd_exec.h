#ifndef CMD_EXEC_H
#define  CMD_EXEC_H

#include "token.h"

struct cmd_block;

enum if_exec_stat_t{
    IE_NONE = 0,                //out of 'if' block
    IE_WAIT_COND,               //wait condition expression
    IE_SUCCEEDED,                //wait for 'then', execute 'then' block
    IE_FAILED,                  //wait for 'then', execute 'else' block or nothing
    IE_DOING_THEN,              //doing 'then' block until 'else' or 'fi'
    IE_SKIPPING_THEN,           //skipping 'then' block until 'else' or 'fi'
    IE_DOING_ELSE,              //doing 'then' block until 'else' or 'fi'
    IE_SKIPPING_ELSE            //skipping 'then' block until 'else' or 'fi'    
};

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

void setup_child_sighandler();

//parse user input and execute commands
void process_shell_cmds(char *args);

//execute cmd block
void exec_cmd_block(struct cmd_block* cmd_blk);

//execute cmd block and return 1 if any of them have returned success
int exec_condition(struct cmd_block* cmd_blk);

//get user input and return it in buffer
char* get_shell_cmd();
#endif