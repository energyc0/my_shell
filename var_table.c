#include "var_table.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

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
static struct table_node* add_entry(struct table_node* node, struct table_node* new_node);
//allocate struct table_node
static struct table_node* mk_table_node(char* s, int is_global);
//print nodes recursively
static void print_node_rec(const struct table_node* node,int is_local);
//count global vars from node
static int count_global_vars(const struct table_node* node);
//push global vars from node
static int push_global_vars(int vars_count, char** buf, struct table_node* node);
//free node 
static void free_table_node(struct table_node* node);
//free node recursively
static void free_table_node_rec(struct table_node* root);
//return variable of the form "var=value" or NULL if this variable doesn't exist
static struct table_node* var_table_lookup(char* name);
//return a node with the given name
static struct table_node* lookup_rec(char* name, int name_len, struct table_node* node);
//free node's string and copy other attributes
static void update_entry(struct table_node* node,char* s, int is_global);
//delete a node, return root
static struct table_node* delete_node(struct table_node* root, char* s);
//find minimum node from the given root
static struct table_node* find_min_node(struct table_node* root);

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
        struct table_node* new_node = mk_table_node(*p, 1);
        var_table_root = add_entry(var_table_root, new_node);
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
static struct table_node* add_entry(struct table_node* node, struct table_node* new_node){
    if(!node){
        return new_node;
    }
    int ret;
    if((ret = strncmp(node->s,new_node->s, new_node->var_name_len)) == 0){
        update_entry(node, new_node->s, new_node->is_global);
        free_table_node_rec(new_node); //THIS IS NOT RIGHT
    }else if(ret < 0){
        node->left = add_entry(node->left, new_node);
    }else{
        node->right = add_entry(node->right, new_node);
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
    free_table_node_rec(var_table_root);
}
static void free_table_node_rec(struct table_node* node){
    if(node){
        free(node->s);
        free_table_node(node->left);
        free_table_node(node->right);
        free(node);
    }
}

static void free_table_node(struct table_node* node){
    if(node){
        free(node->s);
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
        var_table_root = add_entry(var_table_root, p);
    }
    return p->s + p->var_name_len + 1;
}
//return variable of the form "var=value" or NULL if this variable doesn't exist
static struct table_node* var_table_lookup(char* name){
    return lookup_rec(name,strlen(name), var_table_root);
}

//add a variable in the var_table or update existing
void set_variable(char* name_val, int is_global){
    struct table_node* new_node = mk_table_node(name_val, is_global);
    var_table_root = add_entry(var_table_root, new_node);
}

//delete a variable from the var_table
void unset_variable(char* name){
    var_table_root = delete_node(var_table_root, name);
}
//return a node with the given name
static struct table_node* lookup_rec(char* name, int name_len, struct table_node* node){
    if (node) {
        int ret;
        if((ret = strncmp(node->s, name, name_len)) == 0)
            return node;
        else if(ret < 0)
            return lookup_rec(name,name_len, node->left);
        else
            return lookup_rec(name,name_len, node->right);
    }else{
        return NULL;
    }
}

//free node's string and copy other attributes
static void update_entry(struct table_node* node,char* s, int is_global){
    free(node->s);
    node->s = newstr(s);
    node->var_name_len = strchr(s, '=')- s;
    node->is_global = is_global;
}

//delete a node, return root
static struct table_node* delete_node(struct table_node* root, char* s){
    if(root){
        int ret = strncmp(root->s, s, root->var_name_len);
        if(ret > 0){
            root->left = delete_node(root->left, s);
        }else if(ret < 0){
            root->right = delete_node(root->right, s);
        }else{
            struct table_node* temp;
            if(root->left == NULL){
                temp = root->right;
                free_table_node(root);
                root = temp;
            }else if(root->right == NULL){
                temp = root->left;
                free_table_node(root);
                root = temp;
            }else{
                temp = find_min_node(root->right);
                update_entry(root, temp->s, temp->is_global);
                root->right = delete_node(root->right, temp->s);
            }
        }
    }
    return root;
}

//find minimum node from the given root
static struct table_node* find_min_node(struct table_node* root){
    if(root){
        while (root->left) {
            root = root->left;
        }
    }
    return root;
}