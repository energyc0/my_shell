#ifndef CMD_EXEC_H
#define  CMD_EXEC_H

#include "token.h"

typedef enum cmd_keyword_t{
    CMDNONE,
    CMDIF,
    CMDTHEN,
    CMDELSE,
    CMDFI,
    CMDEXIT
} cmd_keyword_t;

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
void process_cmds(char *args);

//return keyword type of a string
cmd_keyword_t get_keyword_type(char* s);

//execute if statement and change the program state
void if_statement_exec(cmd_t cmd);

//if 'if' keyword found try to execute condition statement and change the program state
int process_if_keyword(cmd_t cmd);

//check validity and change the program state
int process_then_keyword(cmd_t cmd);

//check validity and change the program state
int process_else_keyword(cmd_t cmd);

//check validity and change the program state
void process_fi_keyword(cmd_t cmd);

//choose how to execute a command
void choose_to_exec(cmd_t cmd);

//print syntax error 'unexpected token' and change 'if_state' to IS_NONE
void print_synt_err(cmd_t cmd);

//execute command and return the exit code
enum if_stat_result_t cmd_exec(cmd_t args);

//exit shell and parse a return code
void exit_shell(cmd_t args);

#endif