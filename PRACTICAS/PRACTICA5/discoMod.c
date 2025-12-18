#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#define CAPACITY 3
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")
#define FOREVER false
#define ITERATIONS 5

pthread_mutex_t m;
pthread_cond_t c_full, c_vips;
int in_disco;
int waiting_vips;

int vips_ord;
int norm_ord;
int vips_turn;
int norm_turn;

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

void dance(int id, int isvip) {
	printf("Client [%d] (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip) {
	pthread_mutex_lock(&m);

	printf("Client with id [%d] with VIP status (%s) exiting disco\n", id, VIPSTR(isvip));

	in_disco--;
	waiting_vips += isvip;

	pthread_cond_broadcast(&c_full);

	pthread_mutex_unlock(&m);

	sleep((rand() % 3) + 1);
}

void* client(void* arg) {
	intptr_t arg_int = (intptr_t)arg;
	bool vip = arg_int >> 31;
	int id = vip ? arg_int - (1 << 31) : arg_int;
	int iterations = ITERATIONS;

	printf("Created client with id [%d] with VIP status (%s)\n", id, VIPSTR(vip));

	while (FOREVER || iterations--) {
		if (vip) {
			enter_vip_client(id);
		} else {
			enter_normal_client(id);
		}

		dance(id, vip);
		disco_exit(id, vip);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		perror("Unexpected number of arguments\n");
		return EXIT_FAILURE;
	}

	in_disco = 0;
	waiting_vips = 0;

	vips_ord = 0;
	norm_ord = 0;
	vips_turn = 0;
	norm_turn = 0;

	if (pthread_mutex_init(&m, NULL)) {
		perror("Error on initializing mutex\n");
		return EXIT_FAILURE;
	}

	if (pthread_cond_init(&c_full, NULL) || pthread_cond_init(&c_vips, NULL)) {
		perror("Error on initializing thread condition variables\n");
		return EXIT_FAILURE;
	}

	FILE* vips;

	if (!(vips = fopen(argv[1], "r"))) {
		perror("File couldn't be opened\n");
		return EXIT_FAILURE;
	}

	int num_threads;

	if (fscanf(vips, "%d%*c", &num_threads) == EOF) {
		fprintf(stderr, "Error reading file\n");
		fclose(vips);
		return EXIT_FAILURE;
	}

	int* ifvip;
	pthread_t* threads;

	if (!(threads = malloc(num_threads * sizeof(pthread_t)))) {
		perror("Memory couldn't be allocated\n");
		fclose(vips);
		return EXIT_FAILURE;
	}

	if (!(ifvip = malloc(num_threads * sizeof(int)))) {
		perror("Memory couldn't be allocated\n");
		free(threads);
		fclose(vips);
		return EXIT_FAILURE;
	}

	for (int i = 0; i < num_threads; i++) {
		if (fscanf(vips, "%d%*c", &ifvip[i]) == EOF) {
			fprintf(stderr, "Error reading file\n");
			free(threads);
			free(ifvip);
			fclose(vips);
			return EXIT_FAILURE;
		}

		waiting_vips += ifvip[i];
	}

	for (int i = 0; i < num_threads; i++) {
		if (pthread_create(&threads[i], NULL, client, (void*)(intptr_t)(i | (ifvip[i] << 31)))) {
			perror("Error on creating a new thread\n");
			free(threads);
			free(ifvip);
			fclose(vips);
			return EXIT_FAILURE;
		}
	}

	fclose(vips);

	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			perror("Error on waiting for a thread\n");
			free(threads);
			free(ifvip);
			return EXIT_FAILURE;
		}
	}

	free(threads);
	free(ifvip);

	if (pthread_mutex_destroy(&m)) {
		perror("Error on destroying mutex\n");
		return EXIT_FAILURE;
	}

	if (pthread_cond_destroy(&c_full) || pthread_cond_destroy(&c_vips)) {
		perror("Error on destroying thread condition variables\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}