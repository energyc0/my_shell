#include "cmd_exec.h"
#include "if_stack.h"
#include <limits.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

//cmd_info flags
#define CMD_ERR     1   //is error command
int cmd_info;

//execute command and return the exit code, return if_stat_result_t
enum if_stat_result_t cmd_exec(cmd_t args){
    if (args == NULL || (cmd_info & CMD_ERR)) {
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
int process_if_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) != CMDIF || (get_current_state() == IS_WAIT_THEN)){
        print_synt_err(cmd);
        return 0;
    }else{
        struct shell_state sh_st;
        sh_st.if_state = IS_WAIT_COND;
        sh_st.if_res = ISR_NONE;
        push_if_statement(&sh_st);
        return 1;
    }
}

int process_then_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDTHEN && get_current_result() != ISR_NONE && get_current_state() == IS_WAIT_THEN)
        change_current_state(IS_THEN_BLOCK);
    else
        print_synt_err(cmd);
    return get_current_result() == ISR_SUCCESS;
}

int process_else_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDELSE && get_current_result() != ISR_NONE && get_current_state() == IS_THEN_BLOCK)
        change_current_state(IS_ELSE_BLOCK);
    else
        print_synt_err(cmd);
    return get_current_result() == ISR_FAILURE;
}

void process_fi_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == CMDFI && get_current_result() != ISR_NONE && (get_current_state() == IS_THEN_BLOCK || get_current_state() == IS_ELSE_BLOCK)){
        pop_if_statement();
    }
    else
        print_synt_err(cmd);
}

//execute if statement and change the program state
void if_statement_exec(cmd_t cmd){
    change_current_result(cmd_exec(cmd));
    change_current_state(get_current_result() == ISR_NONE ? IS_WAIT_COND : IS_WAIT_THEN);
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
    return (get_current_state() == IS_NONE ||
     (get_current_state() == IS_THEN_BLOCK && get_current_result()  == ISR_SUCCESS) ||
     (get_current_state()  == IS_ELSE_BLOCK && get_current_result()  == ISR_FAILURE));
}

void choose_to_exec(cmd_t cmd){
    switch (get_keyword_type(*cmd)) {
        case CMDEXIT:       exit_shell(cmd); return;
        case CMDIF:         if(process_if_keyword(cmd)) if_statement_exec(cmd+1); return;      //start if block        *    
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
    cmd_info &= (~CMD_ERR);
}

//print syntax error 'unexpected token', change 'if_state' to IS_NONE and set CMD_ERR flag
void print_synt_err(cmd_t cmd){
    printf("unexpected token '%s'\n", cmd[0]);
    clear_if_stack();
    cmd_info |= CMD_ERR;
}