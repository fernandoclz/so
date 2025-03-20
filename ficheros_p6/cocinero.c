// Bibliotecas necesarias para semáforos, memoria compartida, señales, etc.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>     
#include <sys/types.h>      /* For O_* constants */
#include <signal.h>
#include <semaphore.h>

#define M 10  // Número de raciones que el cocinero agrega al caldero

// Variables globales
int fd = -1;  // Descriptor para la memoria compartida
int finish = 0;  // Bandera para controlar la finalización del programa
sem_t *sem_cocinero = NULL, *sem_salvajes = NULL, *sem = NULL; // Semáforos para sincronización
int *ptr = NULL;  // Puntero a la memoria compartida

// Función para liberar recursos
void borrador(){
    // Desvincula memoria compartida si está inicializada
	if(ptr != NULL){
		munmap(ptr, sizeof(int));
		printf("Memoria desvinculada\n");
	}
    // Cierra y elimina el semáforo del cocinero
	if(sem_cocinero != NULL){
		sem_close(sem_cocinero);
		sem_unlink("/cocinero");
		printf("Semaforo cocinero desvinculado\n");
	}
    // Cierra y elimina el semáforo de los salvajes
	if(sem_salvajes != NULL){
		sem_close(sem_salvajes);
		sem_unlink("/salvajes");
		printf("Semaforo salvajes desvinculado\n");
	}
    // Cierra y elimina un semáforo genérico
	if(sem != NULL){
		sem_close(sem);
		sem_unlink("/semaforo");
		printf("Semaforo normal desvinculado\n");
	}
    // Elimina la memoria compartida si está inicializada
	if(fd != -1){
		shm_unlink("/memoryObj");
		printf("Memoria compartida eliminada\n");
	}
}

// Función para que el cocinero llene el caldero
void putServingsInPot(int servings)
{
	sem_wait(sem_cocinero);  // Bloquea hasta que el semáforo del cocinero esté disponible

	*ptr += servings;  // Agrega raciones al caldero
	printf("Llenando caldero\n");

	sem_post(sem_salvajes);  // Desbloquea a los salvajes para que puedan servirse
}

// Función principal del cocinero
void cook(void)
{
	while(!finish) {  // Bucle principal hasta que se reciba la señal de terminación
		putServingsInPot(M);  // Llama a la función para llenar el caldero
	}
}

// Manejador de señales para finalizar el programa
void handler(int signo)
{
	finish = 1;  // Cambia la bandera para salir del bucle en `cook`
	printf("Senial capturada\n");
}

// Función principal
int main(int argc, char *argv[]){
	struct sigaction senial;  // Estructura para manejar señales

    // Configura el manejador de señales SIGINT y SIGTERM
	senial.sa_handler = handler;
	sigemptyset(&senial.sa_mask);
	senial.sa_flags = 0;
	if(sigaction(SIGINT, &senial, NULL) == -1){
		printf("sigaction INT\n");
		exit(1);
	}
	if(sigaction(SIGTERM, &senial, NULL) == -1){
		printf("sigaction TERM\n");
		exit(1);
	}

    // Crea la memoria compartida
	fd = shm_open("/memoryObj", O_CREAT | O_RDWR, 0600);
	if(fd == -1){
		printf("shm_open\n");
		exit(1);
	}

    // Ajusta el tamaño de la memoria compartida
	if(ftruncate(fd, sizeof(int))== -1){
		printf("ftruncate\n");
		borrador();
		exit(1);
	}
	
    // Mapea la memoria compartida en el espacio de direcciones del proceso
	ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED){
		printf("mmap\n");
		borrador();
		exit(1);
	}
	*ptr = 0;  // Inicializa el valor en memoria compartida

    // Inicializa los semáforos
	sem_cocinero = sem_open("/cocinero", O_CREAT, 0700, 0);
	if(sem_cocinero == SEM_FAILED){
		printf("Fallo semaforo cocinero\n");
		borrador();
		exit(1);
	}
	sem_salvajes = sem_open("/salvajes", O_CREAT, 0700, 0);
	if(sem_salvajes == SEM_FAILED){
		printf("Fallo semaforo salvajes\n");
		borrador();
		exit(1);
	}
	sem = sem_open("/semaforo", O_CREAT, 0700, 1);
	if(sem == SEM_FAILED){
		printf("Fallo semaforo\n");
		borrador();
		exit(1);
	}

	cook();  // Ejecuta el bucle principal del cocinero

    // Limpieza de recursos
	munmap(ptr, sizeof(int));
	shm_unlink("/memoryObj");

	sem_close(sem_cocinero);
	sem_close(sem_salvajes);
	sem_close(sem);

	return 0;
}

