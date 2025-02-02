#include "var_table.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define MAX_VARIABLES_COUNT 256

#define GET_MAX_INT(a,b)(((a) > (b)) ? (a) : (b))

struct table_node{
    struct table_node* left;
    struct table_node* right;
    char* s;
    int is_global;
    int var_name_len;
};

static struct table_node* var_table_root = NULL;

extern char** environ;

//add entry to the table tree, return root; 
//s_len must be the length of the variable name and s must be "var_name=value"
static struct table_node* add_entry(char* s, int s_len, int is_global, struct table_node* node);
//allocate struct table_node
static struct table_node* mk_table_node(char* s, int is_global);
//print nodes recursively
static void print_node_rec(const struct table_node* node,int is_local);
//count global vars from node
static int count_global_vars(const struct table_node* node);
//push global vars from node
static int push_global_vars(int vars_count, char** buf, struct table_node* node);

static void free_table_node(struct table_node* node);
//return variable of the form "var=value" or NULL if this variable doesn't exist
static struct table_node* var_table_lookup(char* name);

static struct table_node* lookup_rec(char* name, int name_len, struct table_node* node);
//free node's string and change other attributes
static void update_entry(char* s, struct table_node* node);

//return 1 if string is a correct assign expression, 0 otherwise
int is_correct_assign(char*s){
    char* temp;
    if(!isdigit(*s) && (temp = strchr(s, '=')) != NULL){
        int len = strlen(s);
        int i = 0;
        for(; i < len;i++) {
            if(!IS_VAR_SYM(s[i]) && s + i != temp)
                break;
        }
        if(i >= len)
            return 1;
    }
    return 0;
}

//setup table before use
void table_setup(){
    char** p = environ;
    while (*p) {
        var_table_root = add_entry(*p,strchr(*p, '=') - *p,1, var_table_root);
        p++;
    }
}

//translate var_table to char**, must call free()
char** get_env_vars(){
    char** env_vars;
    int global_count = count_global_vars(var_table_root);
    env_vars = emalloc((global_count + 1)* sizeof(char*) );

    push_global_vars(0, env_vars, var_table_root);

    return env_vars;
}
//print all global variables in var_table, if is_local then print all the variables 
void print_var_table(int is_local){
    print_node_rec(var_table_root, is_local);
}
//add entry to the table tree, return root
static struct table_node* add_entry(char* s, int s_len,int is_global,  struct table_node* node){
    if(!node){
        return mk_table_node(s, is_global);
    }
    int ret;
    if((ret = strncmp(node->s,s, s_len)) == 0){
        update_entry(s, node);
    }else if(ret > 0){
        node->left = add_entry(s, s_len,is_global, node->left);
    }else{
        node->right = add_entry(s, s_len,is_global, node->right);
    }
    return node;
}

//allocate struct table_node ("var_name=value")
static struct table_node* mk_table_node(char* s, int is_global){
    struct table_node* p = emalloc(sizeof(struct table_node));
    p->var_name_len = strchr(s, '=')-s;
    p->s = emalloc(strlen(s) + 1);
    p->s = strcpy(p->s, s);
    p->left = p->right = NULL;
    p->is_global = is_global;
    return p;
}

//print nodes recursively
static void print_node_rec(const struct table_node* node, int is_local){
    if(node){
        print_node_rec(node->left, is_local);
        if(node->is_global || is_local) 
            printf("%s\n", node->s);
        print_node_rec(node->right, is_local);
    }
}

//count global vars from node
static int count_global_vars(const struct table_node* node){
    if(node)
        return count_global_vars(node->left) + count_global_vars(node->right) + node->is_global;
    return 0;
}

//push global vars from node
static int push_global_vars(int vars_count, char** buf, struct table_node* node){
    if (node) {
        vars_count = push_global_vars(vars_count, buf, node->left);
        if(node->is_global)
            buf[vars_count++] = node->s;
        vars_count = push_global_vars(vars_count, buf, node->right);
    }
    return vars_count;
}

//clean the var_table
void table_cleanup(){
    free_table_node(var_table_root);
}

static void free_table_node(struct table_node* node){
    if(node){
        free(node->s);
        free_table_node(node->left);
        free_table_node(node->right);
        free(node);
    }
}

//get variable value, create empty variable if it doesn't exist 
char* var_table_find(char* name){
    struct table_node* p;
    if((p = var_table_lookup(name)) == NULL){
        char temp_buf[strlen(name) + 2];
        sprintf(temp_buf, "%s=", name);
        p = mk_table_node(temp_buf, 0);
    }
    return p->s + p->var_name_len + 1;
}
//return variable of the form "var=value" or NULL if this variable doesn't exist
static struct table_node* var_table_lookup(char* name){
    return lookup_rec(name,strlen(name), var_table_root);
}

//add a variable in the var_table or update existing
void var_table_add(char* name_val, int is_global){
    var_table_root = add_entry(name_val, strchr(name_val, '=') - name_val, is_global, var_table_root);
}

static struct table_node* lookup_rec(char* name, int name_len, struct table_node* node){
    if (node) {
        int ret;
        if((ret = strncmp(node->s, name, name_len)) == 0)
            return node;
        else if(ret > 0)
            return lookup_rec(name,name_len, node->left);
        else
            return lookup_rec(name,name_len, node->right);
    }else{
        return NULL;
    }
}

//free node's string and change other attributes
static void update_entry(char* s, struct table_node* node){
    free(node->s);
    node->s = newstr(s);
    node->var_name_len = strchr(s, '=')- s;
}