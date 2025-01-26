mysh.out: main.c mysh.c mysh.h cmd_exec.h cmd_exec.c utils.c utils.h token.h token.c
	cc main.c mysh.c cmd_exec.c utils.c token.c -o mysh.out -g