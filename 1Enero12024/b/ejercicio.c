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

void *b(void *arg){
    argumento *numHilo = (argumento *)arg;
    int num = numHilo->numHilo;

    int fd2 = open("output.txt", O_WRONLY);
    int pos = num*5;
    lseek(fd2, pos, SEEK_SET);
    char buffer[6];
    sprintf(buffer, "%d", num*11111);
    if(num != 0)
        write(fd2, buffer, 5);
    else
        write(fd2, "00000", 5);
    free(numHilo);
    pthread_exit(0);
}

int main(void)
{
    int fd1,i,pos;
    char c;

    pthread_t hilos[10];

    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    for (i=0; i < 10; i++) {
        argumento * hiloArg = (argumento*)malloc(sizeof(argumento));
		if(hiloArg == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
        hiloArg->numHilo = i;
        if (pthread_create(&hilos[i], NULL, b, hiloArg) != 0) {
            free(hiloArg);
			exit(EXIT_FAILURE);
        }
    }

    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
    return 0;
}
