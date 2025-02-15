#include "token.h"

struct cmd_block{
    int size;
    int p;
    cmd_arr_t data;
};

//resize the block without calling free()
void block_resize(struct cmd_block* v, unsigned long nmemb);

//push back new cmd into the block, may call exit() if limit exceeded
void block_push_back(struct cmd_block* v, cmd_t cmd);

//clear the block without calling free()
void block_clear(struct cmd_block* v);

//freeing the block
void block_free(struct cmd_block* v);