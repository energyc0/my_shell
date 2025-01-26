#include <stdlib.h>

typedef enum token_type{
    TT_ARG,
    TT_KEYWORD
} token_type;

struct token{
    char* val;
    token_type t;
};

typedef enum cmd_keyword_t{
    CMDIF=1,
    CMDTHEN,
    CMDELSE,
    CMDFI,
    CMDEXIT
} cmd_keyword_t;

//free token ptr by calling free() on val and the token pointer
void free_token(struct token* p);

//allocate new token with TT_ARG token_type and copy string with malloc()
struct token* alloc_arg_token(char* s);

//allocate new token with TT_KEYWORD token type and copy cmd_keyword_t with malloc()
struct token* alloc_key_token(cmd_keyword_t t);

//init token with TT_ARG token_type and copy string with malloc()
void init_arg_token(struct token* p, char* s);

//inittoken with TT_KEYWORD token type and copy cmd_keyword_t with malloc()
void init_key_token(struct token* p, cmd_keyword_t t);

//print token value
void print_token(const struct token* p);

struct token_arr{
    struct token** arr;
    size_t sz;
    size_t p;
};

//push token allocated with alloc_*_token() to the token_arr and allocate new space if needed
void push_token(struct token_arr* p, struct token* t);

//call free() on every token_arr entry and on the array, make arr->sz = 0
void clear_token_arr(struct token_arr* arr);

//allocate token array with NULL end identifier, must call free();
struct token** splitline_tokens(char* arg_buf);