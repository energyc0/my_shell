#include "mysh.h"
#include "cmd_block.h"
#include "cmd_exec.h"
#include "token.h"
#include "var_table.h"
#include "if_state.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define COMMAND_BUFFER_SIZE 256
static struct cmd_block command_buffer;

//get user input, split commands separated with ';'
static cmd_arr_t get_user_command_arr();

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
    char* user_input;
    while ((user_input = get_shell_cmd()) != NULL) {
        process_shell_cmds(user_input);
        free(user_input);
    }
}

void cleanup_shell(){
    table_cleanup();
}


//return next cmd
cmd_t get_next_cmd(){
    static cmd_arr_t input_cmd_buf = NULL;
    if(input_cmd_buf != NULL){
        while (*input_cmd_buf == NULL) {
            input_cmd_buf = get_user_command_arr();
        }
        return *input_cmd_buf++;
    }else{
        input_cmd_buf = get_user_command_arr();
        return get_next_cmd();
    }
}

static cmd_arr_t get_user_command_arr(){
    static cmd_arr_t arr_buf = NULL;
    //clear from previous function call
    free_cmd_arr(arr_buf);
    //get next user input
    arr_buf = splitline_cmd(NULL);
}