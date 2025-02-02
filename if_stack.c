#include "if_stack.h"
#include "cmd_exec.h"
#include <stdio.h>
#include <sys/ucontext.h>

#define STSZ 256
static struct shell_state if_stack[STSZ];
static int st_p = -1;

enum if_state_t get_current_state(){
    if (st_p >= 0) 
        return if_stack[st_p].if_state;
    else
        return IS_NONE;
}
enum if_stat_result_t get_current_result(){
    if(st_p >= 0)
        return if_stack[st_p].if_res;
    else
        return ISR_NONE;
}

void change_current_state(enum if_state_t st){
    if (st_p >= 0) {
        if_stack[st_p].if_state = st;
    }else{
        fprintf(stderr, "if_stack is empty!\n");
    }
}
void change_current_result(enum if_stat_result_t res){
    if (st_p >= 0) {
        if_stack[st_p].if_res = res;
    }else{
        fprintf(stderr, "if_stack is empty!\n");
    }
}

void push_if_statement(struct shell_state* st){
    if(st_p < STSZ-1){
        if_stack[++st_p] = *st;
    }else{
        fprintf(stderr, "if_stack is full!\n");
    }
}

void pop_if_statement(){
    if (st_p >= 0) {
        st_p--;
    }else{
        fprintf(stderr, "if_stack is empty!\n");
    }
}

void clear_if_stack(){
    st_p = -1;
}