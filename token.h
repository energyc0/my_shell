#ifndef TOKEN_H
#define  TOKEN_H

#include <stdlib.h>

typedef char* token_t;
typedef token_t* cmd_t;
typedef cmd_t* cmd_arr_t;

//allocate new token, must call free_token()
token_t mktoken(char* s);

//print token value
void print_token(const token_t p);

struct token_arr{
    token_t* arr;
    size_t sz;
    size_t p;
};

//push token allocated with alloc_*_token() to the token_arr and allocate new space if needed
void push_token(struct token_arr* p, token_t t);

//call free() on every token_arr entry and on the array, make arr->sz = 0
void clear_token_arr(struct token_arr* arr);

//allocate command array, must call free()
cmd_arr_t splitline_cmd(char* cmd_buf);

//allocate cmd_t with NULL end identifier, must call free()
cmd_t splitcmd_tokens(char* cmd_buf);

//alloc new copy of cmd
cmd_t copy_cmd(cmd_t cmd);

//print cmd
void print_cmd(cmd_t cmd);

//free cmd pointer
void free_cmd(cmd_t p);

//free cmd_arr_t pointer
void free_cmd_arr(cmd_arr_t p);

#endif