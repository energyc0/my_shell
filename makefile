mysh.out: main.c mysh.c mysh.h cmd_exec.h cmd_exec.c
	cc main.c mysh.c cmd_exec.c -o mysh.out -g