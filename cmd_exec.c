#include "cmd_exec.h"
#include <limits.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

//cmd_info flags
//#define CMD_SKIP    1   //is command to skip
#define CMD_ERR     1   //is error command
//#define CMD_HKW     4   //has keyword to fulfill a command

struct shell_state{
    enum if_stat_result_t if_res;
    enum if_state_t if_state;
    int cmd_info;               //flags
}sh_st;

//execute command and return the exit code, return if_stat_result_t
enum if_stat_result_t cmd_exec(cmd_t args){
    if (args == NULL || (sh_st.cmd_info & CMD_ERR)) {
        return ISR_NONE;
    }
    __pid_t r = fork();
    if(r == -1){
        perror("fork()");
        r = ISR_NONE;
    }else if(r == 0){
        struct sigaction sgnl;
        memset(&sgnl, 0, sizeof sgnl);
        sgnl.sa_handler = SIG_DFL;
        
        sigaction(SIGINT, &sgnl, NULL);
        sigaction(SIGQUIT, &sgnl, NULL);

        if(execvp(args[0], args)){
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
    }else{
        int stat_loc;
        wait(&stat_loc);
        r = WEXITSTATUS(stat_loc) == 0 ? ISR_SUCCESS : ISR_FAILURE;
    }
    //putchar('\n');
    return r;
}

cmd_keyword_t get_keyword_type(char* s){
    if(strcmp("if", s) == 0)
        return CMDIF;
    else if(strcmp("then", s) == 0 )
        return CMDTHEN; 
    else if(strcmp("fi", s)  == 0)
        return CMDFI;
    else if(strcmp("else", s) == 0)
        return CMDELSE;
    else if(strcmp("exit", s) == 0)
        return CMDEXIT;
    else
        return 0;
}

//if 'if' keyword found try to execute condition statement and change the program state
void process_if_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) != CMDIF || (sh_st.if_state == IS_WAIT_THEN))
        print_synt_err(cmd);
    else
        sh_st.if_state = IS_WAIT_COND;
}

int process_then_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDTHEN && sh_st.if_res != ISR_NONE && sh_st.if_state == IS_WAIT_THEN)
        sh_st.if_state = IS_THEN_BLOCK;
    else
        print_synt_err(cmd);
    return sh_st.if_res == ISR_SUCCESS;
}

int process_else_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDELSE && sh_st.if_res != ISR_NONE && sh_st.if_state == IS_THEN_BLOCK)
        sh_st.if_state = IS_ELSE_BLOCK;
    else
        print_synt_err(cmd);
    return sh_st.if_res == ISR_FAILURE;
}

void process_fi_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDFI && sh_st.if_res != ISR_NONE && (sh_st.if_state == IS_THEN_BLOCK || sh_st.if_state == IS_ELSE_BLOCK)){
        sh_st.if_state = IS_NONE;
        sh_st.if_res = ISR_NONE;
    }
    else
        print_synt_err(cmd);
}

//execute if statement and change the program state
void if_statement_exec(cmd_t cmd){
    sh_st.if_res = cmd_exec(cmd);
    sh_st.if_state = sh_st.if_res == ISR_NONE ? IS_WAIT_COND : IS_WAIT_THEN;
}

//exit shell and parse a return code
void exit_shell(cmd_t args){
    exit((args[0] == NULL || args[1] == NULL) ? 0 : atoi(args[1]));
}
/*
char* get_keyword_str(cmd_keyword_t t){    
    static char* strs[] = {NULL, "IF", "THEN", "ELSE", "FI", "EXIT"};
    return strs[t];
}
*/

int is_in_block(){
    return (sh_st.if_state == IS_NONE ||
     (sh_st.if_state == IS_THEN_BLOCK && sh_st.if_res == ISR_SUCCESS) ||
     (sh_st.if_state == IS_ELSE_BLOCK && sh_st.if_res == ISR_FAILURE));
}

void choose_to_exec(cmd_t cmd){
    switch (get_keyword_type(*cmd)) {
        case CMDEXIT:       exit_shell(cmd); return;
        case CMDIF:         process_if_keyword(cmd); if_statement_exec(cmd+1); return;      //start if block        *    
        case CMDTHEN:       if(!process_then_keyword(cmd))return; cmd++; break;  //                      *   change program state and
        case CMDELSE:       if(!process_else_keyword(cmd))return; cmd++; break;   //                     *    execute a command if exist
        case CMDFI:         process_fi_keyword(cmd); cmd++; break;                //out of 'if' block     *
        case CMDNONE:
        default: break;
    }
    if(is_in_block())
        cmd_exec(cmd);
}

void process_cmds(char* args){
    cmd_arr_t tok_vec = splitline_cmd(args);
    for (cmd_arr_t ptr = tok_vec;*ptr; ptr++) {
        choose_to_exec(*ptr);
    }
    free_cmd_arr(tok_vec);
    sh_st.cmd_info &= (~CMD_ERR);
}

//print syntax error 'unexpected token', change 'if_state' to IS_NONE and set CMD_ERR flag
void print_synt_err(cmd_t cmd){
    printf("unexpected token '%s'\n", cmd[0]);
    sh_st.if_state = IS_NONE;
    sh_st.cmd_info |= CMD_ERR;
}