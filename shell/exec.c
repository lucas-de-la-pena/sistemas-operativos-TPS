#include "exec.h"

void redirect(struct cmd *cmd);

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Answer: If the O_CREAT flag is used, the file
// we might have to supplied mode permissions, and
// the `mode` argument from open(2) will only be applied for
// future accesses ofthe file.
// Does it have to be closed after the execve(2) call?
// It doesn't have to be necessarily closed, it can be closed
// using the O_CLOEXEC flag, but it is not necessary.
// The file descriptor will remina open until the process
// terminates or the file descriptor is closed.
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	return flags & O_CREAT ? open(file, flags, S_IWUSR | S_IRUSR)
	                       : open(file, flags);
}

// executes a command - does not return
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		e = (struct execcmd *) cmd;
		// set_environ_vars(e->argv, e->eargc); Deberia ir con la parte de Benjamin
		int op_result = execvp(e->argv[0], e->argv);
		if (op_result < 0) {
			perror("ERROR: Execvp failed on Exec call");
			free_command(cmd);
			exit(-1);
		}
		break;

	case BACK: {
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		redirect(cmd);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// p = (struct pipecmd *)cmd;
		// int pipes_fd[2];
		// pid_t pid = fork();


		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		break;
	}
	}
}

void
redirect(struct cmd *cmd)
{
	struct execcmd *r = (struct execcmd *) cmd;
	if (strlen(r->in_file) > 0) {
		int fd_in = open_redir_fd(r->in_file, O_RDONLY);
		if (fd_in < 0) {
			perror("ERROR: Failed to open input file");
			free_command(cmd);
			exit(-1);
		}
		dup2(fd_in, STDIN_FILENO);
		close(fd_in);
	}
	if (strlen(r->out_file) > 0) {
		int fd_out =
		        open_redir_fd(r->out_file, O_WRONLY | O_TRUNC | O_CREAT);
		if (fd_out < 0) {
			perror("ERROR: Failed to open output file");
			free_command(cmd);
			exit(-1);
		}
		dup2(fd_out, STDOUT_FILENO);
		close(fd_out);
	}
	if (strlen(r->err_file) > 0) {
		if (strcmp(r->err_file, "&1") == 0) {
			dup2(STDOUT_FILENO, STDERR_FILENO);
		} else {
			int fd_err = open_redir_fd(r->err_file,
			                           O_WRONLY | O_TRUNC | O_CREAT);
			if (fd_err < 0) {
				perror("ERROR: Failed to open error file");
				free_command(cmd);
				exit(-1);
			}
			dup2(fd_err, STDERR_FILENO);
			close(fd_err);
		};
	}

	cmd->type = EXEC;
	exec_cmd((struct execcmd *) cmd);
}
