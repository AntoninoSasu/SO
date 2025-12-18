#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")

int inside = 0;                 // Número de clientes dentro de la discoteca
int waiting_vip = 0;            // Número de clientes VIP esperando
int waiting_normal = 0;         // Número de clientes normales esperando

pthread_mutex_t mutex;   //Asegura que sólo un hilo pueda acceder a la sección crítica (modificación de variables compartidas) a la vez
pthread_cond_t cond_vip; // Variable de condición para que los clientes VIP esperen hasta que puedan entrar.
pthread_cond_t cond_normal; //Variable de condición para que los clientes normales esperen hasta que puedan entrar.

void enter_normal_client(int id)
{
    pthread_mutex_lock(&mutex); //Bloquea el mutex para asegurar acceso exclusivo a las variables compartidas
    waiting_normal++;  // Incrementa el número de normales esperando

    // Mientras la discoteca esté llena o haya VIPs esperando, esperar
    while (inside >= CAPACITY || waiting_vip > 0)
    {
        printf("Client %2d (not vip) waiting to enter\n", id);
        pthread_cond_wait(&cond_normal, &mutex); //Entra en espera en cond_normal
    }

    waiting_normal--;  // Decrementa el número de normales esperando
    inside++;          // Incrementa el número de clientes dentro
    printf("Client %2d (not vip) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex); //desbloquea
}

void enter_vip_client(int id)
{
    pthread_mutex_lock(&mutex); //Bloquea el mutex para asegurar acceso exclusivo a las variables compartida
    waiting_vip++;  // Incrementa el número de VIPs esperando

    // Mientras la discoteca esté llena, esperar
    while (inside >= CAPACITY)
    {
        printf("Client %2d (  vip  ) waiting to enter (full capacity)\n", id);
        pthread_cond_wait(&cond_vip, &mutex);
    }

    waiting_vip--;  // Decrementa el número de VIPs esperando
    inside++;       // Incrementa el número de clientes dentro
    printf("Client %2d (  vip  ) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex);  //desbloquea
}


void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
	pthread_mutex_lock(&mutex);

    inside--;  // Decrementa el número de clientes dentro
    printf("Client %2d (%s) exited the disco (inside: %d)\n", id, VIPSTR(isvip), inside);

    // Priorizar la señalización a los VIPs
    if (waiting_vip > 0)
    {
        pthread_cond_signal(&cond_vip); //i hay clientes VIP esperando, señaliza cond_vip para despertar a uno de ellos
    }
    else if (waiting_normal > 0)
    {
        pthread_cond_signal(&cond_normal); //Si no hay VIPs esperando pero hay clientes normales, señaliza cond_normal
    }

    pthread_mutex_unlock(&mutex); //desbloquea

}

void *client(void *arg)
{
	 int id = ((int *)arg)[0];
    int isvip = ((int *)arg)[1];

    if (isvip)
        enter_vip_client(id);
    else
        enter_normal_client(id);

    dance(id, isvip);
    disco_exit(id, isvip);

    pthread_exit(NULL);

}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <fichero_entrada>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r"); //Abre el fichero en modo lectura.
    if (!file)
    {
        perror("Error al abrir el fichero");
        exit(EXIT_FAILURE);
    }

    int M;
    fscanf(file, "%d", &M); //Lee el número total de clientes a crear (M).

    pthread_t threads[M];
    int args[M][2];  // Almacena id y isvip para cada hilo //Matriz para almacenar los argumentos (id y isvip) de cada hilo

	pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_vip, NULL);
    pthread_cond_init(&cond_normal, NULL);

    for (int i = 0; i < M; i++)
    {
        int isvip_char;
        fscanf(file, "%d", &isvip_char);
        args[i][0] = i + 1;         // id del cliente
        args[i][1] = isvip_char == 1;  // isvip (1 o 0)

        pthread_create(&threads[i], NULL, client, (void *)args[i]);
    }

    fclose(file);

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < M; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destruir mutex y variables de condición
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_vip);
    pthread_cond_destroy(&cond_normal);

    return 0;
}

