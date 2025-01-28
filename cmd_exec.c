#include "cmd_exec.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int cmd_exec(cmd_t args){
    __pid_t r = fork();
    if(r == -1){
        perror("fork()");
    }else if(r == 0){
        struct sigaction sgnl;
        memset(&sgnl, 0, sizeof sgnl);
        sgnl.sa_handler = SIG_DFL;
        
        sigaction(SIGINT, &sgnl, NULL);
        sigaction(SIGQUIT, &sgnl, NULL);

        if(execvp(args[0], args)){
            perror(args[0]);
            r = -1;
        }
    }else{
        int stat_loc;
        wait(&stat_loc);
        r = WEXITSTATUS(stat_loc);
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
    else
        return 0;
}

cmd_arr_t if_statement_exec(cmd_arr_t cmd_arr){
    if(get_keyword_type(**cmd_arr) != CMDIF){
        printf("unexpected token '%s'\n", **cmd_arr);
        return NULL;
    }

    return NULL;
}


/*
char* get_keyword_str(cmd_keyword_t t){    
    static char* strs[] = {NULL, "IF", "THEN", "ELSE", "FI", "EXIT"};
    return strs[t];
}
*/