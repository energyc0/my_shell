#include "cmd_exec.h"
#include "if_stack.h"
#include "token.h"
#include "utils.h"
#include "var_table.h"
#include <limits.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
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
static int process_then_keyword(cmd_t cmd);

//check validity and change the program state
static int process_else_keyword(cmd_t cmd);

//check validity and change the program state
static void process_fi_keyword(cmd_t cmd);

//choose how to execute a command
static void choose_to_exec(cmd_t cmd);

//seek for variable and replace them with their values, must call free()
static char* replace_variables(char*);

//print syntax error 'unexpected token' and change 'if_state' to IS_NONE
static void print_synt_err(cmd_t cmd);

//execute command and return the exit code
static enum if_stat_result_t cmd_exec(cmd_t args);

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

//cmd_info flags
#define CMD_ERR     1   //is error command
static int cmd_info;
static char cmd_buf[BUFSIZ];

//get user input, must call free()
char* get_shell_cmd(){

    printf(PROMPT);fflush(stdin);
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return NULL;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return replace_variables(cmd_buf);
}

//execute command and return the exit code, return if_stat_result_t
static enum if_stat_result_t cmd_exec(cmd_t args){
    if (*args == NULL || (cmd_info & CMD_ERR)) {
        return ISR_NONE;
    }
    __pid_t r = fork();
    if(r == -1){
        perror("fork()");
        return ISR_NONE;
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
        int stat_loc;
        wait(&stat_loc);
        r = WEXITSTATUS(stat_loc) == 0 ? ISR_SUCCESS : ISR_FAILURE;
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
    if(get_keyword_type(cmd[0]) != C_IF || (get_current_state() == IS_WAIT_THEN)){
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

static int process_then_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == C_THEN && get_current_result() != ISR_NONE && get_current_state() == IS_WAIT_THEN)
        change_current_state(IS_THEN_BLOCK);
    else
        print_synt_err(cmd);
    return get_current_result() == ISR_SUCCESS;
}

static int process_else_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == C_ELSE && get_current_result() != ISR_NONE && get_current_state() == IS_THEN_BLOCK)
        change_current_state(IS_ELSE_BLOCK);
    else
        print_synt_err(cmd);
    return get_current_result() == ISR_FAILURE;
}

static void process_fi_keyword(cmd_t cmd){
    if(get_keyword_type(cmd[0]) == C_FI && get_current_result() != ISR_NONE && (get_current_state() == IS_THEN_BLOCK || get_current_state() == IS_ELSE_BLOCK))
        pop_if_statement();
    else
        print_synt_err(cmd);
}

//execute if statement and change the program state
static void if_statement_exec(cmd_t cmd){
    change_current_result(cmd_exec(cmd));
    change_current_state(get_current_result() == ISR_NONE ? IS_WAIT_COND : IS_WAIT_THEN);
}

//exit shell and parse a return code
static void exit_shell(cmd_t args){
    exit((args[0] == NULL || args[1] == NULL) ? 0 : atoi(args[1]));
}

static int is_in_block(enum if_state_t st, enum if_stat_result_t res){
    return (st == IS_NONE ||
     (st == IS_THEN_BLOCK && res  == ISR_SUCCESS) ||
     (st == IS_ELSE_BLOCK && res  == ISR_FAILURE));
}

static void choose_to_exec(cmd_t cmd){
    switch (get_keyword_type(*cmd)) {
        case C_EXIT:       exit_shell(cmd); return;
        case C_IF:         if(process_if_keyword(cmd)) cmd++; break;;               //push 'if' block into stack        *   
        case C_THEN:       if(!process_then_keyword(cmd)) return; cmd++; break;     //skip exec if is not in block      *   change program state and
        case C_ELSE:       if(!process_else_keyword(cmd)) return; cmd++; break;     //skip exec if is not in block      *   execute a command if exist
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
    if(get_current_state() == IS_WAIT_COND)
            if_statement_exec(cmd);
    else if(is_in_block(get_current_state(), get_current_result())) {
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