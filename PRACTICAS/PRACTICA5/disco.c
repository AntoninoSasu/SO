#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define CAPACITY 3
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")
#define FOREVER false
#define ITERATIONS 5
#define NUM_DANCEFLOORS 2

pthread_mutex_t m;
pthread_cond_t c_full, c_vips;
int in_disco;
int waiting_vips;

int vips_ord;
int norm_ord;
int vips_turn;
int norm_turn;

pthread_mutex_t dancefloor_m[NUM_DANCEFLOORS];
pthread_cond_t dancefloor_c[NUM_DANCEFLOORS];
int clients_on_dancefloor[NUM_DANCEFLOORS];

typedef struct {
	int id;
	bool is_vip;
	int dancefloor_id;
} client_thread_args;

void enter_normal_client(int id) {
	pthread_mutex_lock(&m);

	int turn = norm_ord++;

	if (waiting_vips != 0) {
		printf("Normal client with id [%d] waiting to enter because of VIPs\n", id);
	}
	
	while ((waiting_vips != 0) || (turn > norm_turn)) {
		pthread_cond_wait(&c_vips, &m);
	}

	if (in_disco == CAPACITY) {
		printf("Normal client with id [%d] waiting to enter because of capacity\n", id);
	}
	
	printf("Normal client waiting with turn (%d), normal turn is (%d)\n", turn, norm_turn);

	while ((in_disco == CAPACITY) || (turn > norm_turn)) {
		pthread_cond_wait(&c_full, &m);
	}
	norm_turn++;

	printf("Normal client with id [%d] entering disco\n", id);
	in_disco++;

	pthread_mutex_unlock(&m);
}

void enter_vip_client(int id) {
	pthread_mutex_lock(&m);

	int turn = vips_ord++;

	if (in_disco == CAPACITY) {
		printf("VIP client with id [%d] waiting to enter because of capacity\n", id);
	}
	
	printf("VIP client waiting with turn (%d), VIPs turn is (%d)\n", turn, vips_turn);

	while ((in_disco == CAPACITY) || (turn > vips_turn)) {
		pthread_cond_wait(&c_full, &m);
	}
	vips_turn++;

	printf("VIP client with id [%d] entering disco\n", id);
	in_disco++;
	waiting_vips--;

	if (!waiting_vips) {
		printf("No VIPs are waiting\n");
		pthread_cond_broadcast(&c_vips);
	}

	pthread_mutex_unlock(&m);
}

void dance(int id, int isvip, int dancefloor_id) {
	printf("Client [%d] (%s) dancing on Dancefloor %d\n", id, VIPSTR(isvip), dancefloor_id);
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip, int dancefloor_id) {
	pthread_mutex_lock(&dancefloor_m[dancefloor_id]);
	printf("Client [%d] (%s) exits Dancefloor %d\n", id, VIPSTR(isvip), dancefloor_id);
	clients_on_dancefloor[dancefloor_id]--;
	pthread_cond_broadcast(&dancefloor_c[dancefloor_id]);
	pthread_mutex_unlock(&dancefloor_m[dancefloor_id]);

	pthread_mutex_lock(&m);

	printf("Client with id [%d] with VIP status (%s) exits the disco\n", id, VIPSTR(isvip));

	in_disco--;

	pthread_cond_broadcast(&c_full);

	pthread_mutex_unlock(&m);

	sleep((rand() % 3) + 1);
}

void* client(void* arg) {
	client_thread_args* args = (client_thread_args*)arg;
	int id = args->id;
	bool vip = args->is_vip;
	int dancefloor_id = args->dancefloor_id;
	int iterations = ITERATIONS;

	printf("Created client with id [%d] with VIP status (%s) and assigned to Dancefloor %d\n", id, VIPSTR(vip), dancefloor_id);

	while (FOREVER || iterations--) {
		if (vip) {
			enter_vip_client(id);
		} else {
			enter_normal_client(id);
		}

		pthread_mutex_lock(&dancefloor_m[dancefloor_id]);
		while (clients_on_dancefloor[dancefloor_id] >= 2) {
			printf("Client [%d] (%s) waiting for Dancefloor %d\n", id, VIPSTR(vip), dancefloor_id);
			pthread_cond_wait(&dancefloor_c[dancefloor_id], &dancefloor_m[dancefloor_id]);
		}
		clients_on_dancefloor[dancefloor_id]++;
		printf("Client [%d] (%s) dancing on Dancefloor %d\n", id, VIPSTR(vip), dancefloor_id);
		pthread_mutex_unlock(&dancefloor_m[dancefloor_id]);

		dance(id, vip, dancefloor_id);
		disco_exit(id, vip, dancefloor_id);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		perror("Unexpected number of arguments\n");
		return EXIT_FAILURE;
	}

	srand(time(NULL));

	in_disco = 0;
	waiting_vips = 0;

	vips_ord = 0;
	norm_ord = 0;
	vips_turn = 0;
	norm_turn = 0;

	clients_on_dancefloor[0] = 0;
	clients_on_dancefloor[1] = 0;

	if (pthread_mutex_init(&m, NULL)) {
		perror("Error on initializing mutex\n");
		return EXIT_FAILURE;
	}

	if (pthread_cond_init(&c_full, NULL) || pthread_cond_init(&c_vips, NULL)) {
		perror("Error on initializing thread condition variables\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < NUM_DANCEFLOORS; i++) {
		if (pthread_mutex_init(&dancefloor_m[i], NULL)) {
			perror("Error on initializing dancefloor mutex\n");
			return EXIT_FAILURE;
		}
		if (pthread_cond_init(&dancefloor_c[i], NULL)) {
			perror("Error on initializing dancefloor condition variable\n");
			return EXIT_FAILURE;
		}
	}

	FILE* clients_file;

	if (!(clients_file = fopen(argv[1], "r"))) {
		perror("File couldn't be opened\n");
		return EXIT_FAILURE;
	}

	int num_threads;

	if (fscanf(clients_file, "%d%*c", &num_threads) == EOF) {
		fprintf(stderr, "Error reading file\n");
		fclose(clients_file);
		return EXIT_FAILURE;
	}

	client_thread_args* thread_args;
	pthread_t* threads;

	if (!(threads = malloc(num_threads * sizeof(pthread_t)))) {
		perror("Memory couldn't be allocated\n");
		fclose(clients_file);
		return EXIT_FAILURE;
	}

	if (!(thread_args = malloc(num_threads * sizeof(client_thread_args)))) {
		perror("Memory couldn't be allocated for thread arguments\n");
		free(threads);
		fclose(clients_file);
		return EXIT_FAILURE;
	}

	for (int i = 0; i < num_threads; i++) {
		int is_vip_read;
		int dancefloor_id_read;
		if (fscanf(clients_file, "%d %d%*c", &is_vip_read, &dancefloor_id_read) == EOF) {
			fprintf(stderr, "Error reading file\n");
			free(threads);
			free(thread_args);
			fclose(clients_file);
			return EXIT_FAILURE;
		}

		thread_args[i].id = i;
		thread_args[i].is_vip = (bool)is_vip_read;
		thread_args[i].dancefloor_id = dancefloor_id_read;

		waiting_vips += is_vip_read;
	}

	for (int i = 0; i < num_threads; i++) {
		if (pthread_create(&threads[i], NULL, client, &thread_args[i])) {
			perror("Error on creating a new thread\n");
			free(threads);
			free(thread_args);
			fclose(clients_file);
			return EXIT_FAILURE;
		}
	}

	fclose(clients_file);

	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			perror("Error on waiting for a thread\n");
			free(threads);
			free(thread_args);
			return EXIT_FAILURE;
		}
	}

	free(threads);
	free(thread_args);

	if (pthread_mutex_destroy(&m)) {
		perror("Error on destroying mutex\n");
		return EXIT_FAILURE;
	}

	if (pthread_cond_destroy(&c_full) || pthread_cond_destroy(&c_vips)) {
		perror("Error on destroying thread condition variables\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < NUM_DANCEFLOORS; i++) {
		if (pthread_mutex_destroy(&dancefloor_m[i])) {
			perror("Error on destroying dancefloor mutex\n");
			return EXIT_FAILURE;
		}
		if (pthread_cond_destroy(&dancefloor_c[i])) {
			perror("Error on destroying dancefloor condition variable\n");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}