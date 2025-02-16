#include "mysh.h"
#include "cmd_block.h"
#include "cmd_exec.h"
#include "token.h"
#include "var_table.h"
#include "if_state.h"
#include "user_input.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define COMMAND_BUFFER_SIZE 256
static struct cmd_block command_buffer;

void setup_shell(){
    struct sigaction sgnl;
    memset(&sgnl, 0, sizeof sgnl);

    sgnl.sa_flags = SA_RESTART;
    sgnl.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sgnl, NULL);
    sigaction(SIGQUIT, &sgnl, NULL);

    block_resize(&command_buffer, COMMAND_BUFFER_SIZE);
    setup_if_state_buffer();
    setup_child_sighandler();
    table_setup();
}

void run_shell(){
    cmd_t cmd;
    while ((cmd = get_next_cmd()) != NULL) {
        choose_to_exec(cmd);
    }
}

void cleanup_shell(){
    table_cleanup();
}