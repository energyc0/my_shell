#include "utils.h"
#include <stdio.h>
#include <string.h>

void* emalloc(size_t size){
    void* p = malloc(size);
    if(p == NULL){
        perror("malloc()");
        exit(EXIT_FAILURE);
    }
    return p;
}
char* newstr(char* s){
    char* p = emalloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}
void* erealloc(void* p,size_t size){
    p = realloc(p, size);
    if (p == NULL) {
        perror("realloc()");
        exit(EXIT_FAILURE);
    }
    return p;
}

void* ecalloc(unsigned long nmemb, size_t size){
    void* ptr;
    if((ptr = calloc(nmemb,size)) == NULL){
        perror("calloc()");
        exit(EXIT_FAILURE);
    }
    return ptr;
}