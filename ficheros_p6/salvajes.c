#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>     
#include <sys/types.h>      /* For O_* constants */
#include <semaphore.h>
// for i in $(seq 10); do ./salvajes; done
#define NUMITER 3  // Número de iteraciones en que los salvajes comen
sem_t *sem_cocinero, *sem_salvajes, *sem; // Semáforos globales
int *ptr;  // Puntero a la memoria compartida

// Funciones
int getServingsFromPot(void)
{
	sem_wait(sem);  // Bloquea acceso al caldero
	if(*ptr == 0){
		printf("No queda comida\n");
		sem_post(sem_cocinero);  // Notifica al cocinero para que rellene
		sem_wait(sem_salvajes);  // Espera a que el cocinero termine
	}

	printf("Me sirvo, quedan %d platos\n", *ptr);
	if(*ptr > 0)
		*ptr -=1;  // Reduce la cantidad de comida
	else
		printf("Error al servirme\n");

	sem_post(sem);  // Libera el acceso al caldero
	return 0;
}

void eat(void)
{
	unsigned long id = (unsigned long) getpid();
	printf("Savage %lu eating\n", id);  // Mensaje indicando que el salvaje está comiendo
	sleep(rand() % 5);  // Simula tiempo para comer
}

void savages(void)
{
	for(int i = 0; i < NUMITER; i++){  // Cada salvaje come varias veces
		getServingsFromPot();  // Obtiene comida del caldero
		eat();  // Simula que come
	}
}

int main(int argc, char *argv[])
{
	int fd;
	fd = shm_open("/memoryObj", O_RDWR, 0);  // Abre la memoria compartida
	if(fd == -1){
		printf("shm_open\n");
		exit(1);
	}

	ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // Mapea la memoria compartida
	if(ptr == MAP_FAILED){
		printf("mmap\n");
		exit(1);
	}

	sem_cocinero = sem_open("/cocinero", 0);  // Abre el semáforo del cocinero
	if(sem_cocinero == SEM_FAILED){
		printf("Fallo semaforo cocinero\n");
		exit(1);
	}
	sem_salvajes = sem_open("/salvajes", 0);  // Abre el semáforo de los salvajes
	if(sem_salvajes == SEM_FAILED){
		printf("Fallo semaforo salvajes\n");
		exit(1);
	}
	sem = sem_open("/semaforo", 0);  // Abre el semáforo genérico
	if(sem == SEM_FAILED){
		printf("Fallo semaforo\n");
		exit(1);
	}

	savages();  // Ejecuta el comportamiento de los salvajes

	munmap(ptr, sizeof(int));  // Desvincula la memoria compartida

	sem_close(sem_cocinero);  // Cierra los semáforos
	sem_close(sem_salvajes);
	sem_close(sem);

	return 0;
}
