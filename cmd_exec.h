typedef char** cmd_to_exec; 

int is_if_keyword(char* s);
char** if_statement_exec(char** args);

int cmd_exec(cmd_to_exec args);

void free_arglist(char** args);