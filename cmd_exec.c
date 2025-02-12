#include "cmd_exec.h"
#include "if_stack.h"
#include "token.h"
#include "utils.h"
#include "var_table.h"
#include <complex.h>
#include <limits.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define PROMPT ">"
#define MAX_VARIABLE_IN_CMD 256

extern char** environ;

//execute if statement and change the program state
static void if_statement_exec(cmd_t cmd);

//if 'if' keyword found try to execute condition statement and change the program state
static int process_if_keyword(cmd_t cmd);

//check validity and change the program state
static void process_then_keyword(cmd_t cmd);

//check validity and change the program state
static void process_else_keyword(cmd_t cmd);

//check validity and change the program state
static void process_fi_keyword(cmd_t cmd);

//choose how to execute a command
static void choose_to_exec(cmd_t cmd);

//seek for variable and replace them with their values, must call free()
static char* replace_variables(char*);

//print syntax error 'unexpected token' and change 'if_state' to IS_NONE
static void print_synt_err(cmd_t cmd);

//execute command and return the exit code
static enum if_state_t cmd_exec(cmd_t args);

//return keyword type of a string
static cmd_type_t get_cmd_type(char* s);

//exit shell and parse a return code
static void exit_shell(cmd_t args);

//delete variables from the table
static void unset_table_vars(cmd_t cmd);

//make variables from the table global
static void export_table_vars(cmd_t cmd);

//read a variable from the user input
static void read_variable(cmd_t cmd);

//SIGCHLD handler
static void sigchld_handler(int code);

//cmd_info flags
#define CMD_ERR     1   //is error command
static int cmd_info;
static char cmd_buf[BUFSIZ];

static int children_count = 0;

//get user input, must call free()
char* get_shell_cmd(){

    printf(PROMPT);fflush(stdin);
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return NULL;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return replace_variables(cmd_buf);
}

//execute command and return the exit code, return IS_SUCCEEDED or IS_FAILED
static enum if_state_t cmd_exec(cmd_t args){
    if (args[0] == NULL || (cmd_info & CMD_ERR)) {
        return IS_FAILED;
    }

    int background_mode;
    cmd_t p = args;
    while (p[1]) {
        p++;
    }
    if(strcmp("&", p[0]) == 0){
        background_mode = 1;
        free(p[0]);
        p[0] = NULL;
    }else{
        background_mode = 0;
    }

    __pid_t r = fork();
    if(r == -1){
        perror("fork()");
        return IS_FAILED;
    }
    if(r == 0){
        struct sigaction sgnl;
        memset(&sgnl, 0, sizeof sgnl);
        sgnl.sa_handler = SIG_DFL;
        
        sigaction(SIGINT, &sgnl, NULL);
        sigaction(SIGQUIT, &sgnl, NULL);
        
        environ = get_env_vars();
        if(execvp(args[0], args)){
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
    }else{
        children_count++;
        //wait for execution or not
        if(!background_mode || get_current_state() == IS_WAIT_COND){
            int stat_loc;
            wait(&stat_loc);
            r = WEXITSTATUS(stat_loc) == 0 ? IS_SUCCEEDED : IS_FAILED;
        }else{
            printf("[%d] %d\n", children_count, r);
            r = IS_SUCCEEDED;
        }
    }

    return r;
}

static cmd_type_t get_keyword_type(char* s){
    if(strcmp("if", s) == 0)
        return C_IF;
    else if(strcmp("then", s) == 0 )
        return C_THEN; 
    else if(strcmp("fi", s)  == 0)
        return C_FI;
    else if(strcmp("else", s) == 0)
        return C_ELSE;
    else if(strcmp("exit", s) == 0)
        return C_EXIT;
    else if(strcmp("env", s) == 0)
        return C_ENV;
    else if(strcmp("set", s) == 0)
        return C_SET;
    else if(strcmp("unset", s) == 0)
        return C_UNSET;
    else if(strcmp("export", s) == 0)
        return C_EXPORT;
    else if(strcmp("read", s) == 0)
        return C_READ;
    else if(is_correct_assign(s))
        return C_ASSIGN;
    return C_NONE;
}

//if 'if' keyword found try to execute condition statement and change the program state
static int process_if_keyword(cmd_t cmd){
    enum if_state_t st = get_current_state();
    if(get_keyword_type(cmd[0]) != C_IF || st == IS_WAIT_COND || st == IS_SUCCEEDED || st == IS_FAILED){
        print_synt_err(cmd);
        return 0;
    }else{
        push_if_statement(IS_WAIT_COND);
        return 1;
    }
}

static void process_then_keyword(cmd_t cmd){
    enum if_state_t st = get_current_state();
    if(get_keyword_type(cmd[0]) == C_THEN && (st == IS_FAILED || st == IS_SUCCEEDED))
        change_current_state(st == IS_SUCCEEDED ? IS_DOING_THEN : IS_SKIPPING_THEN);
    else
        print_synt_err(cmd);
}

static void process_else_keyword(cmd_t cmd){
    enum if_state_t st = get_current_state();
    if(get_keyword_type(cmd[0]) == C_ELSE && (st == IS_SKIPPING_THEN || st == IS_DOING_THEN))
        change_current_state(st == IS_SKIPPING_THEN ? IS_DOING_ELSE : IS_SKIPPING_ELSE);
    else
        print_synt_err(cmd);
}

static void process_fi_keyword(cmd_t cmd){
    enum if_state_t st = get_current_state();
    if(get_keyword_type(cmd[0]) == C_FI && (IS_SKIPPING_ELSE >= st && st >= IS_DOING_THEN))
        pop_if_statement();
    else
        print_synt_err(cmd);
}

//execute if statement and change the program state
static void if_statement_exec(cmd_t cmd){
    change_current_state(cmd_exec(cmd));
}

//exit shell and parse a return code
static void exit_shell(cmd_t args){
    exit((args[0] == NULL || args[1] == NULL) ? 0 : atoi(args[1]));
}

// /media/energyc0/sdb2/programming/repos/unix_programming/chapter-8/waitdemo2.out 3
static int is_skip_block(){
    enum if_state_t st = get_current_state();
    if(st == IS_SKIPPING_THEN || st == IS_SKIPPING_ELSE)
        return 1;
    else
        return 0;
}

static void choose_to_exec(cmd_t cmd){
    switch (get_keyword_type(*cmd)) {
        case C_EXIT:       exit_shell(cmd); return;
        case C_IF:         if(process_if_keyword(cmd)) cmd++; break;;               //push 'if' block into stack        *   
        case C_THEN:       process_then_keyword(cmd); cmd++; break;     //skip exec if is not in block      *   change program state and
        case C_ELSE:       process_else_keyword(cmd); cmd++; break;     //skip exec if is not in block      *   execute a command if exist
        case C_FI:         process_fi_keyword(cmd); cmd++; break;                   //pop 'if' block                    *
        case C_ASSIGN:     set_variable(*cmd, 0); cmd++; break;
        case C_SET:        print_var_table(1); return;
        case C_UNSET:      unset_table_vars(cmd); return;
        case C_ENV:        print_var_table(0); return;
        case C_EXPORT:     export_table_vars(cmd); return;
        case C_READ:       read_variable(cmd); return;
        case C_NONE:
        default: break;
    }
    if(!is_skip_block()){
        if(get_current_state() == IS_WAIT_COND)
            if_statement_exec(cmd);
        else 
            cmd_exec(cmd);
    }
}

void process_shell_cmds(char* args){
    cmd_arr_t tok_vec = splitline_cmd(args);
    for (cmd_arr_t ptr = tok_vec;*ptr; ptr++) {
        choose_to_exec(*ptr);
    }
    free_cmd_arr(tok_vec);
    cmd_info &= (~CMD_ERR);
}

//print syntax error 'unexpected token', change 'if_state' to IS_NONE and set CMD_ERR flag
static void print_synt_err(cmd_t cmd){
    printf("unexpected token '%s'\n", cmd[0]);
    clear_if_stack();
    cmd_info |= CMD_ERR;
}

//seek for variable and replace them with their values, must call free()
static char* replace_variables(char* cmd_buf){
    char* p = cmd_buf;
    char* var_name_start;
    char* found_variables[MAX_VARIABLE_IN_CMD];
    int var_count = 0;
    int new_length = strlen(cmd_buf);

    //search variables in buffer and add them to the 'found_variables' array, calculate new length
    while ((var_name_start = strchr(p, '$')) != NULL) {
        if(*++var_name_start == '\0')
            break;
        
        if(var_count >= MAX_VARIABLE_IN_CMD){
            fprintf(stderr, "variables in command limit exceeded!\n");
            exit(EXIT_FAILURE);
        }

        char* var_name_end;
        for (var_name_end = var_name_start; IS_VAR_SYM(*var_name_end); var_name_end++);

        char temp = *var_name_end;
        *var_name_end = '\0';
        found_variables[var_count] = var_table_find(var_name_start);
        new_length += (strlen(found_variables[var_count++]) - (var_name_end - var_name_start)) - 1;
        *var_name_end = temp;
        p = var_name_end;
    }

    //allocate new string and replace all the variables with 'found_variables' array
    char* replaced_str = emalloc(new_length + 1);
    memset(replaced_str, '\0', new_length+1);
    char* insert_ptr = replaced_str;
    p = cmd_buf;
    int i = 0;
    while ((var_name_start = strchr(p, '$')) != NULL) {
        insert_ptr = strncat(insert_ptr, p, var_name_start-p);
        if(*++var_name_start == '\0')
            break;
        for (; IS_VAR_SYM(*var_name_start); var_name_start++);
        strcat(insert_ptr, found_variables[i++]);
        p = var_name_start;
    }
    strcat(insert_ptr, p);
    replaced_str[new_length] = '\0';
    return replaced_str;
}

//delete variables from the table
static void unset_table_vars(cmd_t cmd){
    while(*++cmd != NULL)
        unset_variable(*cmd);
}

//make variables from the table global
static void export_table_vars(cmd_t cmd){
    while(*++cmd != NULL)
        export_variable(*cmd);
}

static void read_variable(cmd_t cmd){
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        perror("fgets()");
        exit(EXIT_FAILURE);
    }
    int len = strlen(cmd_buf);

    cmd_buf[len-1]='\0';
    char temp[len + strlen(cmd[1]) + 2];
    sprintf(temp, "%s=%s", cmd[1], cmd_buf);
    set_variable(temp, 0);
}

static void sigchld_handler(int code){
    int p_status;
    pid_t id;
    while ((id = waitpid(-1, &p_status, WNOHANG)) > 0) {
        printf("%d exited with code %d.\n", id, WEXITSTATUS(p_status));
        children_count--;
    }
}

void setup_child_sighandler(){
    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof sgnl);
    sgnl.sa_handler = sigchld_handler;
    sgnl.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sgnl, NULL)){
        perror("sigaction");
        exit(EXIT_FAILURE);    
    }
}