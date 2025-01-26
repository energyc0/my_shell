#include "cmd_exec.h"
#include "mysh.h"
#include "token.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

char cmd_buf[BUFSIZ];

void setup_shell(){
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

int get_cmd(){
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return 0;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return 1;
}

void process_cmd(){
    struct token** tok_vec = splitline_tokens(cmd_buf);
    struct token** ptr = tok_vec;
    while (*ptr) {
        print_token(*ptr);
        free_token(*ptr++);
    }
    free(tok_vec);
    /*char** args = arg_separator(cmd_buf);
    if (args[0] == NULL)
        return;

    int ret;
    if((ret = is_if_keyword(args[0]))){
        if_statement_exec(args);
    }else{
        int res = cmd_exec(args);
        printf("exited with status: %d\n", res);
    }
    free_arglist(args);
    */



}