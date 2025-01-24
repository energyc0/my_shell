#include "cmd_exec.h"
#include "mysh.h"
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
    char** args = arg_separator(cmd_buf);
    __pid_t r = fork();
    if(r == -1){
        perror("fork()");
    }else if(r == 0){
        cmd_exec(args);
    }else{
        wait(NULL);
    }
}