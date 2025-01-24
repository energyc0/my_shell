#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define ARG_ALLOC 20

struct arg_buf{
    char** argv;
    size_t p_end;
    size_t sz;
};


void add_arg(struct arg_buf* b, char* s){
    if(b->p_end >= b->sz){
        b->sz+=ARG_ALLOC;
        char** np = realloc(b->argv, b->sz);
        if (np == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }
        b->argv = np;
    }
    b->argv[b->p_end++] = s == NULL ? NULL : strdup(s);
}

char** arg_separator(char* arg_buf){
    struct arg_buf b;
    memset(&b, 0, sizeof b);

    char* p = arg_buf;
    char* prev = arg_buf;
    while (*prev) {
        while (*p != '\0' && *p != ' ') p++;
        char ch = *p;
        *p = '\0';
        add_arg(&b, prev);
        if((*p = ch) == '\0')
            break; 
        prev = ++p;
    }
    add_arg(&b, NULL);
    return b.argv;
}

int cmd_exec(char** args){
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    return execvp(args[0], args);
}