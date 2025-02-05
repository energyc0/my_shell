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
//print all global variables in var_table, if is_local then print all the variables 
void print_var_table(int is_local);

//get variable value, create empty variable if it doesn't exist 
char* var_table_find(char* name);
//add a variable in the var_table or update existing
void set_variable(char* name_val, int is_global);
//delete a variable from the var_table
void unset_variable(char* name);

//clean the var_table
void table_cleanup();
#endif