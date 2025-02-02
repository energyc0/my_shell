#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include <ctype.h>

#define IS_VAR_SYM(ch)(isalnum(ch) || ch == '_')

//return 1 if string is a correct assign expression, 0 otherwise
int is_correct_assign(char*s);

//setup table before use
void table_setup();
//translate var_table to char**, must call free()
char** get_env_vars();
//'set' command
void print_var_table();

//get variable value, create empty variable if it doesn't exist 
char* var_table_find(char* name);
//count length of all strings and create a big one and add it to var_table
int var_table_try_add(char** name_val);

//clean the var_table
void table_cleanup();
#endif