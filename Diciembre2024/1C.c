#include <stdio.h>   // Librería estándar para operaciones de entrada/salida
#include <stdlib.h>  // Librería estándar para gestión de memoria dinámica y utilidades
#include <unistd.h>  // Librería para funciones como sleep()
#include <pthread.h> // Librería para trabajar con hilos
#include <sched.h>   // Librería para funciones de planificación

// Definición de constantes y macros
#define CAPACITY 5                                  // Capacidad máxima de la discoteca
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip") // Convierte un booleano VIP a texto descriptivo

// Estructura para almacenar información de cada cliente
typedef struct
{
    int numCliente; // Identificador del cliente
    int vip;        // Bandera que indica si el cliente es VIP (1 para VIP, 0 para no VIP)
} info;

// Variables globales
int turno_actual_vip = 0;       // Turno actual para clientes VIP
int turno_actual = 0;           // Turno actual para clientes normales
int dispensador_turnos = 0;     // Contador para asignar turnos a clientes normales
int dispensador_turnos_vip = 0; // Contador para asignar turnos a clientes VIP
int vips_esperando = 0;         // Número de VIPs esperando para entrar
int clientes_esperando = 0;     // Número de clientes normales esperando para entrar
int clientes_dentro = 0;        // Número de clientes actualmente dentro de la discoteca

// Sincronización
pthread_mutex_t m;         // Mutex para proteger las variables compartidas
pthread_cond_t espera;     // Variable de condición para clientes normales
pthread_cond_t espera_vip; // Variable de condición para clientes VIP

// EX.
int clean_pending = 0;
int terminar = 0;
int clientes_han_entrado = 0;
pthread_cond_t espera_limpieza;

// EX. Entrada de limpieza
void enter_cleaning()
{
    pthread_mutex_lock(&m);
    printf("Cleaning thread waiting to enter the disco. \n");

    while (((clientes_han_entrado == 0) || (clientes_dentro > 0)) && clean_pending == 0 && terminar == 0)
    {
        if (clientes_han_entrado == 3)
        {
            clean_pending = 1;
        }
        pthread_cond_wait(&espera_limpieza, &m);
    }
    pthread_mutex_unlock(&m);
}

// EX. Salida de limpieza()
void exit_cleaning()
{
    pthread_mutex_lock(&m);

    clean_pending = 0;
    clientes_han_entrado = 0;

    // Notifica a los clientes en espera
    if (vips_esperando > 0)
    {
        pthread_cond_broadcast(&espera_vip); // Despierta a todos los VIPs en espera
    }
    else if (clientes_esperando > 0)
    {
        pthread_cond_broadcast(&espera); // Despierta a los clientes normales en espera
    }

    pthread_mutex_unlock(&m);
}

void *cleaning_thread(void *arg)
{
    while (terminar == 0)
    {
        enter_cleaning();

        printf("Cleaning...\n");
        sleep(2);
        printf("Done!\n");
        exit_cleaning();
    }
    return NULL;
}

// Función para que un cliente normal intente entrar en la discoteca
void enter_normal_client(int id)
{
    pthread_mutex_lock(&m); // Bloquea el mutex para garantizar acceso exclusivo

    // Obtiene el turno para el cliente actual
    int turno = dispensador_turnos++;
    clientes_esperando++; // Incrementa el contador de clientes esperando

    // Espera si:
    // - La discoteca está llena
    // - Hay VIPs en espera
    // - No es el turno actual del cliente

    while (clientes_dentro >= CAPACITY || vips_esperando > 0 || turno != turno_actual || clean_pending)
    {
        printf("Client %2d (not vip) waiting.\n", id);
        pthread_cond_wait(&espera, &m); // Espera en la variable de condición
    }

    // Actualiza el estado al entrar
    turno_actual++;       // Incrementa el turno actual
    clientes_esperando--; // Disminuye los clientes en espera
    clientes_dentro++;    // Incrementa los clientes dentro
    // EX
    clientes_han_entrado++;
    pthread_cond_broadcast(&espera_limpieza);


    printf("Client %2d (not vip) entered. Occupancy %d\n", id, clientes_dentro);

    pthread_mutex_unlock(&m); // Libera el mutex
}

// Función para que un cliente VIP intente entrar en la discoteca
void enter_vip_client(int id)
{
    pthread_mutex_lock(&m); // Bloquea el mutex para garantizar acceso exclusivo

    // Obtiene el turno VIP para el cliente actual
    int turno = dispensador_turnos_vip++;
    vips_esperando++; // Incrementa el contador de VIPs esperando

    // Espera si:
    // - La discoteca está llena
    // - No es el turno actual del cliente VIP
    while (clientes_dentro >= CAPACITY || turno_actual_vip != turno || clean_pending)
    {
        printf("Client %2d (  vip  ) waiting.\n", id);
        pthread_cond_wait(&espera_vip, &m); // Espera en la variable de condición VIP
    }

    // Actualiza el estado al entrar
    turno_actual_vip++; // Incrementa el turno VIP actual
    vips_esperando--;   // Disminuye los VIPs en espera
    clientes_dentro++;  // Incrementa los clientes dentro
    // EX
    clientes_han_entrado++;
    pthread_cond_broadcast(&espera_limpieza);


    printf("Client %2d (  vip  ) entered. Occupancy %d.\n", id, clientes_dentro);

    pthread_mutex_unlock(&m); // Libera el mutex
}

// Función para simular que el cliente está bailando
void dance(int id, int isvip)
{
    printf("Client %2d (%s) dancing in disco.\n", id, VIPSTR(isvip));
    sleep((rand() % 3) + 1); // Simula un tiempo de baile aleatorio (1-3 segundos)
}

// Función para que un cliente salga de la discoteca
void disco_exit(int id, int isvip)
{
    pthread_mutex_lock(&m); // Bloquea el mutex para garantizar acceso exclusivo

    clientes_dentro--; // Disminuye el contador de clientes dentro
    printf("Client %2d (%s) left. Occupancy %d.\n", id, VIPSTR(isvip), clientes_dentro);

    if (clean_pending && clientes_dentro == 0)
        pthread_cond_broadcast(&espera_limpieza);
    // Notifica a los clientes en espera
    if (vips_esperando > 0)
    {
        pthread_cond_broadcast(&espera_vip); // Despierta a todos los VIPs en espera
    }
    else if (clientes_esperando > 0)
    {
        pthread_cond_broadcast(&espera); // Despierta a los clientes normales en espera
    }

    pthread_mutex_unlock(&m); // Libera el mutex
}

// Función que ejecuta cada cliente (hilo)
void *client(void *arg)
{
    info *argumentos = (info *)arg;  // Castea el argumento recibido a la estructura info
    int isvip = argumentos->vip;     // Determina si el cliente es VIP
    int id = argumentos->numCliente; // Obtiene el ID del cliente
    free(argumentos);                // Libera la memoria dinámica asignada

    // Intenta entrar según si es VIP o no
    if (isvip)
    {
        enter_vip_client(id);
    }
    else
    {
        enter_normal_client(id);
    }

    dance(id, isvip);      // Simula que el cliente está bailando
    disco_exit(id, isvip); // El cliente sale de la discoteca

    pthread_exit(0); // Finaliza el hilo
}

// Función principal
int main(int argc, char *argv[])
{
    // Verifica si se pasó un archivo de entrada como argumento
    if (argc != 2)
    {
        printf("No hay suficientes argumentos para %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fichero;

    // Abre el archivo de entrada
    if ((fichero = fopen(argv[1], "r")) == NULL)
    {
        printf("Error al abrir el fichero de entrada %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Inicializa el mutex y las variables de condición
    if (pthread_mutex_init(&m, NULL) != 0)
    {
        printf("Error iniciar mutex\n");
        fclose(fichero);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&espera, NULL) != 0)
    {
        printf("Error iniciar variable condicion\n");
        pthread_mutex_destroy(&m);
        fclose(fichero);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&espera_vip, NULL) != 0)
    {
        printf("Error iniciar variable condicion vip\n");
        pthread_cond_destroy(&espera);
        pthread_mutex_destroy(&m);
        fclose(fichero);
        exit(EXIT_FAILURE);
    }
    // Lee el número total de clientes
    int clientes;
    if (fscanf(fichero, "%d", &clientes) != 1)
    {
        printf("Error al leer el fichero de entrada\n");
        pthread_cond_destroy(&espera);
        pthread_mutex_destroy(&m);
        pthread_cond_destroy(&espera_vip);
        fclose(fichero);
        exit(EXIT_FAILURE);
    }

    pthread_t hilos[clientes]; // Arreglo para almacenar los identificadores de los hilos

    // EX. Inicializo espera_limpieza
    if (pthread_cond_init(&espera_limpieza, NULL) != 0)
    {
        printf("Error al leer el fichero de entrada\n");
        pthread_cond_destroy(&espera);
        pthread_mutex_destroy(&m);
        pthread_cond_destroy(&espera_vip);
        fclose(fichero);
        exit(EXIT_FAILURE);
    }

    // EX. Creo el hilo del servicio de limpieza

    pthread_t hilo_limpieza;

    if (pthread_create(&hilo_limpieza, NULL, cleaning_thread, NULL) != 0)
    {
        perror("pthread_create");
        pthread_cond_destroy(&espera);
        pthread_mutex_destroy(&m);
        pthread_cond_destroy(&espera_vip);
        pthread_cond_destroy(&espera_limpieza);
        fclose(fichero);
        exit(EXIT_FAILURE);
    }

    // Crea un hilo para cada cliente
    for (int i = 0; i < clientes; i++)
    {
        int vip;
        if (fscanf(fichero, "%d", &vip) != 1)
        {
            printf("Fichero más grande de lo esperado\n");
            pthread_cond_destroy(&espera);
            pthread_mutex_destroy(&m);
            pthread_cond_destroy(&espera_vip);
            pthread_cond_destroy(&espera_limpieza);

            fclose(fichero);
            exit(EXIT_FAILURE);
        }

        // Asigna memoria dinámica para los argumentos del hilo
        info *argumentos = (info *)malloc(sizeof(info));
        if (!argumentos)
        {
            perror("malloc argumentos");
            pthread_cond_destroy(&espera);
            pthread_mutex_destroy(&m);
            pthread_cond_destroy(&espera_vip);
            pthread_cond_destroy(&espera_limpieza);

            fclose(fichero);
            exit(EXIT_FAILURE);
        }

        argumentos->numCliente = i + 1; // Asigna el número del cliente
        argumentos->vip = vip;          // Asigna si el cliente es VIP

        // Crea el hilo
        if (pthread_create(&hilos[i], NULL, client, argumentos) != 0)
        {
            perror("pthread_create");
            free(argumentos);
            pthread_cond_destroy(&espera);
            pthread_mutex_destroy(&m);
            pthread_cond_destroy(&espera_vip);
            pthread_cond_destroy(&espera_limpieza);

            fclose(fichero);
            exit(EXIT_FAILURE);
        }
    }

    fclose(fichero); // Cierra el archivo de entrada

    // Espera a que terminen todos los hilos
    for (int i = 0; i < clientes; i++)
    {
        if (pthread_join(hilos[i], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_lock(&m);
    terminar = 1;
    pthread_cond_broadcast(&espera_limpieza);
    pthread_mutex_unlock(&m);

    // Libera los recursos de sincronización
    pthread_join(hilo_limpieza, NULL); // Cancela el hilo de limpieza
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&espera);
    pthread_cond_destroy(&espera_vip);
    pthread_cond_destroy(&espera_limpieza);

    printf("Discoteca cerrada.\n");

    return 0; // Fin del programa
}
