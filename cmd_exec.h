#ifndef CMD_EXEC_H
#define  CMD_EXEC_H

#include "token.h"

int is_if_keyword(char* s);
char** if_statement_exec(char** args);

int cmd_exec(cmd_t args);

void free_arglist(char** args);

#endif