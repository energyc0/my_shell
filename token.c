#include "token.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//allocate token array with NULL end identifier, must call free();
struct token** splitline_tokens(char* arg_buf){
    char* prev = arg_buf;
    char* p = arg_buf;

    struct token_arr arr;
    memset(&arr, 0, sizeof arr);

    for (; *p ; p++) {
        while(*p != ' ' && *p != '\0') p++;
        
        char ch = *p;
        *p = '\0';
        push_token(&arr, alloc_arg_token(prev));
        if((*p = ch) == '\0')
            break;

        prev = p+1;
    }

    push_token(&arr, NULL);
    return arr.arr;
}

//free token ptr by calling free() on val and the token pointer
void free_token(struct token* t){
    free(t->val);
    free(t);
}

//allocate new token with TT_KEYWORD token type and copy cmd_keyword_t with malloc()
struct token* alloc_key_token(cmd_keyword_t t){
    struct token* ptr = emalloc(sizeof(struct token));
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


#define TOKEN_ALLOC_CHUNK 20

void realloc_token_arr(struct token_arr* p){
    p->sz += TOKEN_ALLOC_CHUNK;
    p->arr = erealloc(p->arr, p->sz * sizeof(struct token));
}
void push_token(struct token_arr* ptr, struct token* t){
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
void print_token(const struct token* p){
    if (p == NULL) {
        fprintf(stderr, "token is NULL!\n");
    }else{
        switch (p->t) {
            case TT_ARG: printf("%s\n", p->val); break;
            case TT_KEYWORD: printf("%s\n", get_keyword_str(*p->val)); break;
            default: fprintf(stderr, "undefined token type!\n");break;
        }
    }
}