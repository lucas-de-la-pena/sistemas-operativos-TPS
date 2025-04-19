#include "builtin.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define COMANDO_NO_CORRESPONDE 0

int
exit_shell(char *cmd)
{
    if (strncmp(cmd, "exit", 4) != 0
        || (cmd[4] != '\0' && cmd[4] != ' '))           
        return COMANDO_NO_CORRESPONDE;

    exit(EXIT_SUCCESS);
}

int
cd(char *cmd)
{
    if (strncmp(cmd, "cd", 2) != 0
        || (cmd[2] != '\0' && cmd[2] != ' '))           
        return COMANDO_NO_CORRESPONDE;

    char *arg = cmd + 2;
    while (*arg == ' ') arg++;                     

    if (*arg == '\0') {                            
        arg = getenv("HOME");
        if (!arg) {
            perror("cd: no se pudo obtener el directorio HOME");
            return 1;
        }
    }

    if (chdir(arg) == -1) {                        
        perror("cd: no se pudo cambiar de directorio");
    }
    return 1;
}

int
pwd(char *cmd)
{
	if (strncmp(cmd, "pwd", 3) != 0
	|| (cmd[3] != '\0' && cmd[3] != ' '))           
	return COMANDO_NO_CORRESPONDE;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)))
        printf("%s\n", cwd);
    else
        perror("pwd: no se pudo obtener el directorio actual");
    return 1;
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
