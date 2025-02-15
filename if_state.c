#include "if_state.h"
#include "cmd_exec.h"
#include "token.h"
#include "cmd_block.h"
#include <stdio.h>
#include <stdlib.h>

#define CMD_ARRAY_SIZE 256

struct current_if_state{
    enum if_state_t st;
    struct cmd_block if_cond;
    struct cmd_block then_block;
    struct cmd_block else_block;
} cur_stat;

//print error and reset if state
static void print_state_error(cmd_t cmd);

//return current if_state
enum if_state_t get_if_state(){
    return cur_stat.st;
}

//changing if_state without checking errors
void change_if_state(enum if_state_t st){
    cur_stat.st = st;
}

//return 1 if cmd is in 'if' condition block, 'then' or 'else' block, 0 otherwise
int preserve_cmd(cmd_t cmd, cmd_type_t t){
    if(t == C_IF){
        if(cur_stat.st != IS_NONE){
            print_state_error(cmd);
            return 1;
        }
        cur_stat.st = IS_IF_COND;
        cmd++;
    }else if(t == C_THEN){
        if(cur_stat.st != IS_IF_COND){
            print_state_error(cmd);
            return 1;
        }
        cur_stat.st = IS_THEN_BLOCK;
        cmd++;
    }else if(t == C_ELSE){
        if(cur_stat.st != IS_THEN_BLOCK){
            print_state_error(cmd);
            return 1;
        }
        cur_stat.st = IS_ELSE_BLOCK;
        cmd++;
    }else if(t == C_FI){
        if(cur_stat.st != IS_THEN_BLOCK && cur_stat.st != IS_ELSE_BLOCK){
            print_state_error(cmd);
            return 1;
        }
        cur_stat.st = IS_NONE;

        if(exec_condition(&cur_stat.if_cond))
            exec_cmd_block(&cur_stat.then_block);
        else
            exec_cmd_block(&cur_stat.else_block);
        reset_if_state();
    }
    switch (cur_stat.st) {
        case IS_IF_COND:    block_push_back(&cur_stat.if_cond, cmd); break;
        case IS_THEN_BLOCK: block_push_back(&cur_stat.then_block, cmd); break;
        case IS_ELSE_BLOCK: block_push_back(&cur_stat.else_block, cmd); break;
        case IS_NONE:       return 0;
        default:            fprintf(stderr, "undefined if_state\n"); exit(EXIT_FAILURE);
    }
    return 1;
}

//call on setup
void setup_if_state_buffer(){
    cur_stat.st = IS_NONE;
    block_resize(&cur_stat.if_cond, CMD_ARRAY_SIZE);
    block_resize(&cur_stat.then_block, CMD_ARRAY_SIZE);
    block_resize(&cur_stat.else_block, CMD_ARRAY_SIZE);
}

//set the state out of 'if' statement and reset command buffers
void reset_if_state(){
    cur_stat.st = IS_NONE;
    block_clear(&cur_stat.if_cond);
    block_clear(&cur_stat.then_block);
    block_clear(&cur_stat.else_block);
}

//print error and reset if state
static void print_state_error(cmd_t cmd){
    fprintf(stderr, "unexpected token '%s'\n", cmd[0]);
    reset_if_state();
}