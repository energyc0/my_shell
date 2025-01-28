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

//return keyword type of a string
cmd_keyword_t get_keyword_type(char* s);

//execute if statement and return pointer to the next command
cmd_arr_t if_statement_exec(cmd_arr_t cmd_arr);

//execute command and return the exit code
int cmd_exec(cmd_t cmd);

#endif