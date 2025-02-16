#include "token.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_COMMANDS_PER_LINE 64
#define TOKEN_ALLOC_CHUNK 20

struct token_arr{
    token_t* arr;
    size_t sz;
    size_t p;
};

//push token allocated with alloc_*_token() to the token_arr and allocate new space if needed
static void push_token(struct token_arr* p, token_t t);
static void realloc_token_arr(struct token_arr* p);

//call free() on every token_arr entry , make arr->p = 0
static void clear_token_arr(struct token_arr* arr);

//return token array with NULL end identifier, must call free()
static cmd_t splitcmd_tokens(char* cmd_buf);

//return next command array
cmd_arr_t splitline_cmd(char* arg_buf){
    static cmd_arr_t cmds = NULL;
    char* prev = arg_buf;
    char* p = arg_buf;

    free_cmd_arr(cmds);
    if(cmds == NULL)
        cmds = emalloc(sizeof(cmd_arr_t) * MAX_COMMANDS_PER_LINE);
    int i = 0;

    while (1) {
        while (*p != '\0' && (isspace(*p) || *p == ';')) p++;
        if(*p == '\0')
            break;
        prev = p;
        while (*p != '\0' && *p != ';') p++;

        if(i >= MAX_COMMANDS_PER_LINE-1)
            break;
        char ch = *p;
        *p = '\0';
        cmds[i++] = splitcmd_tokens(prev);
        if((*p = ch) == '\0')
            break;

        prev = ++p;
    }
    cmds[i] = NULL;
    return cmds;
}

//allocate new token, must call free_token()
token_t mktoken(char* s){
    token_t ptr = newstr(s);
    return ptr;
}

//print token value
void print_token(const token_t p){
    if (p == NULL) {
        fprintf(stderr, "token is NULL!\n");
    }else{
        printf("%s", p);
    }
}
//print cmd
void print_cmd(cmd_t cmd){
    while (*cmd) {
        print_token(*cmd++);
        putchar(' ');
    }
}
//free cmd_t entries
void free_cmd(cmd_t p){
    while(*p){
        FREE_TOKEN(*p++);
    }
}
//free cmd_arr_t entries
void free_cmd_arr(cmd_arr_t p){
    if(p){
        while(*p){
            free_cmd(*p++);
        }
    }
}
//alloc new copy of cmd
cmd_t copy_cmd(cmd_t cmd){
    int sz = 0;
    while (cmd[sz]) {
        sz++;
    }
    cmd_t ptr = emalloc(sizeof(token_t) * (sz+1));
    for(int i = 0; i < sz; i++){
        ptr[i] = newstr(cmd[i]);
    }
    ptr[sz] = NULL;
    return ptr;
}

//return token array with NULL end identifier, must call free()
static cmd_t splitcmd_tokens(char* cmd_buf){
    char* p = cmd_buf;
    struct token_arr arr;
    memset(&arr,0,sizeof arr);

    for (char* prev; *p ; prev = p) {
        while(*p != '\0' && isspace(*p)) p++;
        prev = p;
        while(*p != '\0' && !isspace(*p)) p++;

        char ch = *p;
        *p = '\0';
        push_token(&arr, mktoken(prev));
        if((*p++ = ch) == '\0')
            break;

    }

    push_token(&arr, NULL);
    return arr.arr;
}

static void realloc_token_arr(struct token_arr* p){
    p->sz += TOKEN_ALLOC_CHUNK;
    p->arr = erealloc(p->arr, p->sz * sizeof(token_t));
}
static void push_token(struct token_arr* ptr, token_t t){
    if(ptr->p >= ptr->sz){
        realloc_token_arr(ptr);
    }

    ptr->arr[ptr->p++] = t;
}

//call free() on every token_arr entry , make arr->p = 0
static void clear_token_arr(struct token_arr* arr){
    for(int i = 0; i < arr->p; i++){
        free(arr->arr[i]);
    }
    arr->p = 0;
}