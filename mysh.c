#include "mysh.h"
#include "cmd_exec.h"
#include "var_table.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

void setup_shell(){
    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof sgnl);

    sgnl.sa_flags = SA_RESTART;
    sgnl.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sgnl, NULL);
    sigaction(SIGQUIT, &sgnl, NULL);

    //table_setup();
}

void run_shell(){
    char* user_input;
    while ((user_input = get_cmd()) != NULL) {
        process_cmds(user_input);
        free(user_input);
    }
}

void cleanup_shell(){
    table_cleanup();
}