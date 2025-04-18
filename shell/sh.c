#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

#define STACK_SIZE (16 * 1024)  // 16kb
static char altstack_buffer[STACK_SIZE];  // Buffer estatico para la pila alternativa global

char prompt[PRMTLEN] = { 0 };


// Handler para los background proccess
void
handler_bp(int sig)
{
	pid_t pid_bp;

	// Verifico tener el mismo gpid actual con el recibido
	if ((pid_bp = waitpid(0, &status, WNOHANG)) <= 0)
		return;

	char buffer[BUFLEN];
	int len = snprintf(
	        buffer, sizeof(buffer), "\r==> terminado: PID=%d\n$ \n", pid_bp);
	write(STDOUT_FILENO, buffer, len);  // Atomicidad -> Uso write
}


// runs a shell command
static void
run_shell()
{
	char *cmd;

	stack_t altstack;  // Stack alternativo
	altstack.ss_sp = altstack_buffer;
	altstack.ss_size = STACK_SIZE;
	altstack.ss_flags = SS_ONSTACK;  // Seteo uso de la pila alt

	if (sigaltstack(&altstack, NULL) == -1) {  // Inicializacion pila alt
		perror("Sigaltstack error");
		exit(1);
	}

	struct sigaction sa;
	sa.sa_handler = handler_bp;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK |
	              SA_RESTART;  // Handler usa pila alt y reinicia la flag

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("Sigaction error");
		exit(1);
	}

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
