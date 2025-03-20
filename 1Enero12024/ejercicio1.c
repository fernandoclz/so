#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>    // Librería para trabajar con hilos

typedef struct{
    int numHilo;
}argumento;

void *a(void *arg){
    argumento *numHilo = (argumento *)arg;
    int num = numHilo->numHilo;
    printf("Hilo : %d\n", num);
    free(numHilo);
    pthread_exit(0);
}

int main(void)
{
    int fd1,fd2,i,pos;
    char c;

    pthread_t hilos[10];

    for (i=0; i < 10; i++) {
        argumento * hiloArg = (argumento*)malloc(sizeof(argumento));
		if(hiloArg == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
        hiloArg->numHilo = i;
        if (pthread_create(&hilos[i], NULL, a, hiloArg) != 0) {
            free(hiloArg);
			exit(EXIT_FAILURE);
        }
    }

    return 0;
}
