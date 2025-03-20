#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

typedef struct{
	int numHilo;
	char prio;
}info;


void *thread_usuario(void *arg)
{
	info* argumento = (info*) arg;
	int num = argumento->numHilo;
	char prio = argumento->prio;

	free(arg);

	pthread_t id = pthread_self();

	printf("[ ID : %lu | Hilo N: %d | Prioridad : %c]\n", id, num, prio);
}

int main(int argc, char* argv[])
{
	int n = 6;
	pthread_t hilos[n];
	pthread_attr_t atributos;
	struct sched_param param;

	pthread_attr_init(&atributos); // Inicializo atributos
	pthread_attr_setschedpolicy(&atributos, SCHED_FIFO); //Pongo FIFO
	for(int i = 0; i < n; i++){
		info * hiloArg = (info*)malloc(sizeof(info));
		if(hiloArg == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		hiloArg->numHilo = i;
		hiloArg->prio = ((i%2)==0) ? 'P' : 'N';

		param.sched_priority = ((i%2)==0) ? 80 : 20;

		pthread_attr_setschedparam(&atributos, &param);

		if(pthread_create(&hilos[i], &atributos, thread_usuario, hiloArg) != 0){
			perror("pthread_create");
			free(hiloArg);
			exit(EXIT_FAILURE);
		}
	}

	for(int i = 0; i < n; i++){
		pthread_join(hilos[i], NULL);
	}

	pthread_attr_destroy(&atributos);

	return 0;
}
