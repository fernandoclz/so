#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>    // Librería para trabajar con hilos
#include <sched.h>      // Librería para funciones de planificación

int turno_escritura = 0;
int dispensador_de_turnos = 0;
int esperando = 0;

pthread_mutex_t m;
pthread_cond_t cond;

typedef struct{
    int numHilo;
    int fd;
}argumento;


void escribir(int num, int fd1){

    pthread_mutex_lock(&m);

    int turno = dispensador_de_turnos++;
    esperando++;

    if(turno != turno_escritura){
        printf("Hilo %d espera\n", num);
        pthread_cond_wait(&cond, &m);
    }
    char buffer[6];
    sprintf(buffer, "%d", num*11111);
    if(num != 0){
        write(fd1, buffer, 5);
    }
    else{
        write(fd1, "00000", 5);
    }

    turno_escritura++;
    esperando--;
    pthread_mutex_unlock(&m);
}

void termina(){
    pthread_mutex_lock(&m);
    if(esperando > 0){
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&m);
}

void *a(void *arg){
    argumento *numHilo = (argumento *)arg;
    int num = numHilo->numHilo;
    int fd1 = numHilo->fd;
    escribir(num, fd1);
    printf("Hilo %d ha escrito\n" , num);
    termina();
    
    free(numHilo);
    pthread_exit(0);
}

int main(void)
{
    int fd1,i,pos;
    char c;

    pthread_t hilos[10];

    if(pthread_mutex_init(&m, NULL)!=0){
		printf("Error iniciar mutex\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&cond, NULL)!=0){
		printf("Error iniciar variable condicion\n");
		pthread_mutex_destroy(&m);
		exit(EXIT_FAILURE);
	}

    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    lseek(fd1, 0, SEEK_SET);
    for (i=0; i < 10; i++) {
        argumento * hiloArg = (argumento*)malloc(sizeof(argumento));
		if(hiloArg == NULL){
			perror("malloc");
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&m);
            close(fd1);
			exit(EXIT_FAILURE);
		}
        hiloArg->numHilo = i;
        hiloArg->fd = fd1;
        if (pthread_create(&hilos[i], NULL, a, hiloArg) != 0) {
            perror("Error al crear hilo");
            free(hiloArg);
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&m);
            close(fd1);
			exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 10; i++) {
        if (pthread_join(hilos[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&m);
    close(fd1);
    return 0;
}
