#include "builtin.h"
#include "utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define SPACE_CHAR ' '
#define TERM_CHAR '\0'
#define ENV_HOME "HOME"

#define BUILTIN_PROCESADO 1
#define COMANDO_NO_CORRESPONDE 0

#define EXIT_CMD "exit"

#define CD_CMD "cd"
#define CD_CMD_ERR_MSG "cd: no se pudo cambiar al directorio '%s'\n"
#define CD_CMD_ERR_MSG_NO_HOME "cd: no se pudo obtener el directorio HOME\n"

#define PWD_CMD "pwd"
#define PWD_CMD_ERR_MSG "pwd: no se pudo obtener el directorio actual\n"

#define CD_CMD_LEN 2

#define ERR_CHDIR -1
#define ERR_GETCWD -1


int
command_matches(char *cmd, char *name)
{
	int len = strlen(name);
	return strncmp(cmd, name, len) == 0 &&
	       (cmd[len] == TERM_CHAR || cmd[len] == SPACE_CHAR);
}

int
exit_shell(char *cmd)
{
	if (!command_matches(cmd, EXIT_CMD))
		return COMANDO_NO_CORRESPONDE;

	exit(EXIT_SUCCESS);
}

int
cd(char *cmd)
{
	if (!command_matches(cmd, CD_CMD))
		return COMANDO_NO_CORRESPONDE;

	char *arg = cmd + CD_CMD_LEN;
	while (*arg == SPACE_CHAR)
		arg++;

	if (*arg == TERM_CHAR) {
		arg = getenv(ENV_HOME);
		if (!arg) {
			fprintf_debug(stderr, CD_CMD_ERR_MSG_NO_HOME);
			return BUILTIN_PROCESADO;
		}
	}

	if (chdir(arg) == ERR_CHDIR) {
		fprintf_debug(stderr, CD_CMD_ERR_MSG, arg);
	}
	return BUILTIN_PROCESADO;
}

int
pwd(char *cmd)
{
	if (!command_matches(cmd, PWD_CMD))
		return COMANDO_NO_CORRESPONDE;

	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)))
		printf("%s\n", cwd);
	else
		fprintf_debug(stderr, PWD_CMD_ERR_MSG);
	return BUILTIN_PROCESADO;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
