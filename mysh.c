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
    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof sgnl);

    sgnl.sa_flags = SA_RESTART;
    sgnl.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sgnl, NULL);
    sigaction(SIGQUIT, &sgnl, NULL);
}

int get_cmd(){
    if(fgets(cmd_buf, BUFSIZ, stdin) == NULL){
        return 0;
    }
    cmd_buf[strlen(cmd_buf)-1] = '\0';
    return 1;
}

void process_cmd(){
    cmd_arr_t* tok_vec = splitline_cmd(cmd_buf);
    cmd_arr_t* ptr = tok_vec;
    while (*ptr) {
        print_cmd(*ptr++);
        putchar('\n');
    }
    free_cmd_arr(tok_vec);


}