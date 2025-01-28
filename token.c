#include "token.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS_PER_LINE 64

//allocate command array, must call free()
cmd_arr_t splitline_cmd(char* arg_buf){
    char* prev = arg_buf;
    char* p = arg_buf;

    cmd_arr_t cmds = emalloc(sizeof(cmd_arr_t) * MAX_COMMANDS_PER_LINE);
    int i = 0;
    while (*p) {
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

//allocate token array with NULL end identifier, must call free();
cmd_t splitcmd_tokens(char* cmd_buf){
    char* p = cmd_buf;

    struct token_arr arr;
    memset(&arr, 0, sizeof arr);

    while(isspace(*p)) p++;
    for (char* prev = p; *p ; prev = p) {
        while(*p != ' ' && *p != '\0') p++;

        char ch = *p;
        *p = '\0';
        push_token(&arr, mktoken(prev));
        if((*p = ch) == '\0')
            break;

        while(isspace(*p)) p++;
    }

    push_token(&arr, NULL);
    return arr.arr;
}

/*
//allocate new token with TT_KEYWORD token type and copy cmd_keyword_t with malloc()
token alloc_key_token(){
    token ptr = emalloc(sizeof(token));
    init_key_token(ptr, t);
    return ptr;
}

//allocate new token with TT_ARG token_type and copy string with malloc()
struct token* alloc_arg_token(char* s){
    struct token* ptr = emalloc(sizeof(struct token));
    init_arg_token(ptr, s);
    return ptr;
}

//init token with TT_ARG token_type and copy string with malloc()
void init_arg_token(struct token* p, char* s){
    p->val = newstr(s);
    p->t = TT_ARG;
}

//inittoken with TT_KEYWORD token type and copy cmd_keyword_t with malloc()
void init_key_token(struct token* p, cmd_keyword_t t){
    p->val = emalloc(sizeof(cmd_keyword_t));
    *p->val = t;
    p->t = TT_KEYWORD;
}
*/

//allocate new token, must call free_token()
token_t mktoken(char* s){
    token_t ptr = newstr(s);
    return ptr;
}

#define TOKEN_ALLOC_CHUNK 20

void realloc_token_arr(struct token_arr* p){
    p->sz += TOKEN_ALLOC_CHUNK;
    p->arr = erealloc(p->arr, p->sz * sizeof(token_t));
}
void push_token(struct token_arr* ptr, token_t t){
    if(ptr->p >= ptr->sz){
        realloc_token_arr(ptr);
    }

    ptr->arr[ptr->p++] = t;
}

void clear_token_arr(struct token_arr* arr){
    for(int i = 0; i < arr->p; i++){
        free(arr->arr[i]);
    }
    free(arr->arr);
    arr->arr = NULL;
    arr->p = 0;
    arr->sz = 0;
}

char* get_keyword_str(cmd_keyword_t t){    
    static char* strs[] = {NULL, "IF", "THEN", "ELSE", "FI", "EXIT"};
    return strs[t];
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

void free_cmd(cmd_t p){
    cmd_t temp = p;
    while(*p){
        FREE_TOKEN(*p++);
    }
    free(temp);
}

void free_cmd_arr(cmd_arr_t p){
    cmd_arr_t temp = p;
        while(*p){
        free_cmd(*p++);
    }
    free(temp);
}