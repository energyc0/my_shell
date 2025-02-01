#include "mysh.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define PROMPT ">"

char cmd_buf[BUFSIZ];

void setup_shell(){
    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof sgnl);

    sgnl.sa_flags = SA_RESTART;
    sgnl.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sgnl, NULL);
    sigaction(SIGQUIT, &sgnl, NULL);
}

char* get_cmd(){
    printf(PROMPT);fflush(stdin);
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return NULL;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return cmd_buf;
}