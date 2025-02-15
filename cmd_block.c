#ifndef CMD_BLOCK_H
#define CMD_BLOCK_H

#include "cmd_block.h"
#include "token.h"
#include "utils.h"
#include <stdio.h>

void block_resize(struct cmd_block* v, unsigned long nmemb){
    v->data = ecalloc(nmemb, sizeof(cmd_t));
    v->size = nmemb;
    v->p = 0;
}

void block_push_back(struct cmd_block* v, cmd_t cmd){
    if(cmd[0]){
        if(v->size > v->p+1){
            v->data[v->p] = copy_cmd(cmd);
            v->data[++v->p] = NULL;    
        }else{
            fprintf(stderr, "commands limit exceeded\n");
            exit(EXIT_FAILURE);
        }
    }
}

void block_clear(struct cmd_block* v){
    v->p = 0;
    v->data[v->p] = NULL;
}

void block_free(struct cmd_block* v){
    block_clear(v);
    v->size = 0;
    free_cmd_arr(v->data);
    free(v->data);
}

#endif