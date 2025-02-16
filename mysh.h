#ifndef MYSH_H
#define MYSH_H

#include "token.h"

void setup_shell();
void run_shell();

//return next cmd, must call free()
cmd_t get_next_cmd();

void cleanup_shell();

#endif