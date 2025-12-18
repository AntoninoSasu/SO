#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")

int inside = 0;
int waiting_vip = 0;
int waiting_normal = 0;

int clean_pending = 0;          // 1 si la limpieza está pendiente o en curso
int clients_since_clean = 0;    // clientes que han entrado desde la última limpieza
int finished = 0;               // 1 cuando ya han terminado todos los clientes

pthread_mutex_t mutex;
pthread_cond_t cond_vip;
pthread_cond_t cond_normal;
pthread_cond_t cond_clean;

/* --- Cleaning --- */

void enter_cleaning(void)
{
    pthread_mutex_lock(&mutex);

    // Esperar hasta que haya al menos 3 clientes desde la última limpieza,
    // o hasta que finished=1 (para poder terminar limpio)
    while (clients_since_clean < 3 && !finished) {
        pthread_cond_wait(&cond_clean, &mutex);
    }

    // Si ya acabaron los clientes, salir sin limpiar
    if (finished) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Bloquea nuevas entradas (cuando toque limpiar)
    clean_pending = 1;

    // Espera a que la disco quede vacía
    while (inside > 0 && !finished) {
        pthread_cond_wait(&cond_clean, &mutex);
    }

    pthread_mutex_unlock(&mutex);
}

void exit_cleaning(void)
{
    pthread_mutex_lock(&mutex);

    clean_pending = 0;
    clients_since_clean = 0;

    // Despertar a clientes esperando (ellos mismos respetan VIP/normal en sus while)
    pthread_cond_broadcast(&cond_vip);
    pthread_cond_broadcast(&cond_normal);

    pthread_mutex_unlock(&mutex);
}

void* cleaning_thread_funcion(void* arg)
{
    (void)arg;

    while (1) {
        enter_cleaning();

        pthread_mutex_lock(&mutex);
        if (finished) {              // ✅ salir “normal”
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        printf("Cleaning...\n");
        sleep(2);
        printf("Done!\n");

        exit_cleaning();
    }
    return NULL;
}

/* --- Clients enter --- */

void enter_normal_client(int id)
{
    pthread_mutex_lock(&mutex);
    waiting_normal++;

    while (inside >= CAPACITY || waiting_vip > 0 || clients_since_clean>=3) {
        printf("Client %2d (not vip) waiting to enter\n", id);
        pthread_cond_wait(&cond_normal, &mutex);
    }

    waiting_normal--;
    inside++;
    clients_since_clean++;

    // ✅ si ya han entrado 3 desde la última limpieza, avisar al limpiador
    if (clients_since_clean >= 3) {
        pthread_cond_signal(&cond_clean);
    }

    printf("Client %2d (not vip) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex);
}

void enter_vip_client(int id)
{
    pthread_mutex_lock(&mutex);
    waiting_vip++;

    while (inside >= CAPACITY || clients_since_clean>=3) {
        printf("Client %2d (  vip  ) waiting to enter\n", id);
        pthread_cond_wait(&cond_vip, &mutex);
    }

    waiting_vip--;
    inside++;
    clients_since_clean++;

    // avisar al limpiador al llegar a 3
    if (clients_since_clean >= 3) {
        pthread_cond_signal(&cond_clean);
    }

    printf("Client %2d (  vip  ) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex);
}

/* --- Dance + Exit --- */

void dance(int id, int isvip, int dancefloor)
{
    printf("Client %2d (%s) dancing in disco with dancefloor %d\n", id, VIPSTR(isvip), dancefloor);
    sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
    pthread_mutex_lock(&mutex);

    inside--;
    printf("Client %2d (%s) exited the disco (inside: %d)\n", id, VIPSTR(isvip), inside);

    // Si queda vacía, puede que el limpiador esté esperando
    if (inside == 0) {
        pthread_cond_signal(&cond_clean);
    }

    // Prioridad VIP
    if (waiting_vip > 0) {
        pthread_cond_signal(&cond_vip);
    } else if (waiting_normal > 0) {
        pthread_cond_signal(&cond_normal);
    }

    pthread_mutex_unlock(&mutex);
}

void *client(void *arg)
{
    int id = ((int *)arg)[0];
    int isvip = ((int *)arg)[1];
    int dancefloor = ((int *)arg)[2];

    if (isvip)
        enter_vip_client(id);
    else
        enter_normal_client(id);

    dance(id, isvip, dancefloor);
    disco_exit(id, isvip);

    return NULL;
}

/* --- Main --- */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <fichero_entrada>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error al abrir el fichero");
        return 1;
    }

    int M;
    if (fscanf(file, "%d", &M) != 1 || M <= 0) {
        fprintf(stderr, "Error leyendo M\n");
        fclose(file);
        return 1;
    }

    pthread_t threads[M];
    int args[M][3];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_vip, NULL);
    pthread_cond_init(&cond_normal, NULL);
    pthread_cond_init(&cond_clean, NULL);

    pthread_t cleaner_thread;
    pthread_create(&cleaner_thread, NULL, cleaning_thread_funcion, NULL);

    for (int i = 0; i < M; i++) {
        int dancefloor, isvip_char;
        if (fscanf(file, "%d ", &isvip_char) != 1) {
            fprintf(stderr, "Error leyendo cliente %d (se esperan 2 ints: isvip y dancefloor)\n", i+1);
            fclose(file);
            return 1;
        }

        args[i][0] = i + 1;
        args[i][1] = (isvip_char == 1);
        args[i][2] = dancefloor;

        pthread_create(&threads[i], NULL, client, (void *)args[i]);
    }

    fclose(file);

    for (int i = 0; i < M; i++) {
        pthread_join(threads[i], NULL);
    }

    // Señalar final al limpiador y despertarlo si está esperando
    pthread_mutex_lock(&mutex);
    finished = 1;
    pthread_cond_broadcast(&cond_clean);
    pthread_mutex_unlock(&mutex);

    pthread_join(cleaner_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_vip);
    pthread_cond_destroy(&cond_normal);
    pthread_cond_destroy(&cond_clean);

    return 0;
}
