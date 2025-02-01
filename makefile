mysh.out: main.c mysh.c mysh.h cmd_exec.h cmd_exec.c utils.c utils.h token.h token.c if_stack.h if_stack.c
	cc main.c mysh.c cmd_exec.c utils.c token.c if_stack.c -o mysh.out -g