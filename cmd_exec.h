enum cmd_keyword_t{
    CMDIF=1,
    CMDTHEN,
    CMDELSE,
    CMDFI,
    CMDEXIT
};

int is_if_keyword(char* s);
char** if_statement_exec(char** args);

char** arg_separator(char* arg_buf);
int cmd_exec(char** args);

void free_arglist(char** args);