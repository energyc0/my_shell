#include "user_input.h"
#include "var_table.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define PROMPT ">"
#define MAX_VARIABLE_IN_CMD 256

//get user input, split commands separated with ';'
static cmd_arr_t get_user_command_arr();
//replace variable names with their values
static char* replace_variables(char* cmd_buf);

//return next cmd
cmd_t get_next_cmd(){
    static cmd_arr_t input_cmd_buf = NULL;

    if(input_cmd_buf == NULL || *input_cmd_buf == NULL)
        if((input_cmd_buf = get_user_command_arr()) == NULL)
            return NULL;
    return *input_cmd_buf++;
}

static cmd_arr_t get_user_command_arr(){
    static cmd_arr_t arr_buf = NULL;
    //get next user input
    
    char* user_input;
    if((user_input = get_user_input()) == NULL)
        return NULL;
    user_input = replace_variables(user_input);
    arr_buf = splitline_cmd(user_input);

    return arr_buf; 
}

//get a line from stdin or NULL if EOF omitted
char* get_user_input(){
    static char cmd_buf[BUFSIZ];

    printf(PROMPT);fflush(stdin);
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return NULL;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return cmd_buf;
}

//replace variable names with their values
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