#ifndef INPUT_H
#define INPUT_H

#include "token.h"

//return next cmd, must call free()
cmd_t get_next_cmd();

//get a line from stdin or NULL if EOF omitted
char* get_user_input();

#endif