#include "mysh.h"
#include "cmd_exec.h"

int main(){
    setup_shell();

    char* user_input;
    while ((user_input = get_cmd()) != 0) {
        process_cmds(user_input);
    }
    return 0;
}