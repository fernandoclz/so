#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t pid = 0;
/*programa que temporiza la ejecución de un proceso hijo */

void manejador_SIGALM(int sig){
	if(pid > 0){
		printf("Killing child %d\n", pid);
		kill(pid, SIGKILL);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		printf("Insuficiente número de argumentos para: %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	struct sigaction sa;

	sa.sa_handler = manejador_SIGALM; //Manejar la señal
	sigemptyset(&sa.sa_mask); //Vaciar la mascara de señales bloqueadas
	sa.sa_flags = 0;   		//Opciones adicionales: 0
	if(sigaction(SIGALRM, &sa, NULL) == -1){
		perror("sigaction ALARM");
		exit(EXIT_FAILURE);
	}

	struct sigaction sa_int;

	sa_int.sa_handler = SIG_IGN; //Para detectar SIGINT
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = 0;
	if(sigaction(SIGINT, &sa_int, NULL) == -1){
		perror("sigaction INT");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if(pid < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0){
		//proceso hijo
		printf("Proceso hijo %d\n", getpid());
		execvp(argv[1], &argv[1]);
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	else{
		printf("Proceso padre\n");
		alarm(5);

		int status;
		waitpid(pid, &status, 0);

		if(WIFEXITED(status)){
			printf("Proceso hijo termina con status: %d\n", WEXITSTATUS(status));
		}
		else if(WIFSIGNALED(status)){
			printf("Proceso hijo interrumpido por %d\n", WTERMSIG(status));
		}
		else{
			printf("¿Error?\n");
		}
	}
	return 0;
}
