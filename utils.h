#ifndef MYSH_UTILS_H
#define  MYSH_UTILS_H

#include <stdlib.h>

void* emalloc(size_t size);
void* ecalloc(unsigned long nmemb, size_t size);
char* newstr(char* s);
void* erealloc(void* p, size_t size);

#endif