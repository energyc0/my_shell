#include "cmd_exec.h"
#include "utils.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define ARG_ALLOC 20

struct arg_buf{
    char** argv;
    size_t p_end;
    size_t sz;
};


void add_arg(struct arg_buf* b, char* s){
    if(b->p_end >= b->sz){
        b->sz+=ARG_ALLOC;
        char** np = erealloc(b->argv, b->sz * sizeof(char*));
        b->argv = np;
    }
    b->argv[b->p_end++] = ((s == NULL) ? NULL : newstr(s));
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

int is_if_keyword(char* s){
    /*if(strcmp("if", s) == 0)
        return CMDIF;
    else if(strcmp("then", s) == 0 )
        return CMDTHEN; 
    else if(strcmp("fi", s)  == 0)
        return CMDFI;
    else if(strcmp("else", s) == 0)
        return CMDELSE;
    else*/
        return 0;
}

char** if_statement_exec(char** args){
    /*char** p = args;
    int ret;
    if((ret = is_if_keyword(*p++)) != CMDIF){
        fprintf(stderr, "'if' expected\n");
        return NULL;
    }
    if((ret = is_if_keyword(*p++))){
        fprintf(stderr, "unexpected token\n");
        return NULL;
    }
    while(*p && !(ret = is_if_keyword(*p))){
        p++;
    }
    if(*p == NULL || ret != CMDTHEN){
        fprintf(stderr, "'then' expected\n");
        return NULL;
    }

    char* temp = *p;
    *p = NULL;
    ret = cmd_exec(args+1);
    *p = temp;

    if(ret == 0){
        args = ++p;
        while(*p && !(ret = is_if_keyword(*p))){
            p++;
        }
        if (ret != CMDFI || *p == NULL) {
            fprintf(stderr, "'fi' expected\n");
            return NULL;
        }
        temp = *p;
        *p = NULL;
        cmd_exec(args);
        *p++ = temp;
    }else{
        while(*p){
            if((ret = is_if_keyword(*p)))
            p++;
        }
    }
    
    return p;
    */
    return NULL;
}

void free_arglist(char** args){
    char** p = args;
    while(*p)
        free(*p++);
    free(args);
}