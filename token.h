#ifndef TOKEN_H
#define  TOKEN_H

#include <stdlib.h>

typedef char* token_t;
typedef token_t* cmd_t;
typedef cmd_t* cmd_arr_t;

//just free()
#define FREE_TOKEN(p) {free(p);}

//allocate new token, must call free_token()
token_t mktoken(char* s);

//print token value
void print_token(const token_t p);

//return next command array
cmd_arr_t splitline_cmd(char* cmd_buf);

//alloc new copy of cmd
cmd_t copy_cmd(cmd_t cmd);

//print cmd
void print_cmd(cmd_t cmd);

//free cmd_t entries
void free_cmd(cmd_t p);

//free cmd_arr_t entries
void free_cmd_arr(cmd_arr_t p);

#endif