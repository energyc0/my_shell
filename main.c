#include "mysh.h"

int main(){
    setup_shell();

    while (get_cmd()) {
        process_cmd();
    }
    return 0;
}