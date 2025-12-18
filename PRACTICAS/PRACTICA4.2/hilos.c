#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// Respuesta a la pregunta:
// La creación de los threads no es necesariamente inmediata,
// por lo que si se usa una misma variable para los argumen-
// tos de todos los threads, habrá threads que tendrán in-
// formación equivocada.

typedef struct {
	int user_id;
	char priority;
} user_t;

void* thread_usuario(void* arg) {

	//conviertes el tipo generico arg void al tipo user_t
	user_t params = *(user_t*)arg;

	free(arg);
	
	printf("This is my id: %lu. This is my user id: %d. This is my priority; %c.\n", pthread_self(), params.user_id, params.priority);

	return NULL;
}

int main(int argc, char* argv[]) {

	if (argc != 2){
		fprintf(stderr, "Usage: %s <number of threads> \n", argv[0]);
    	return EXIT_FAILURE;
	}

	// Pasamos a int el numero de hilos
	int n = strtol(argv[1], NULL, 10);

	int count = 0;
	int wait = 1;
	
	pthread_t* threads = malloc(n * sizeof(pthread_t));

	for (int i = 0; i < n; i++) {
		pthread_t thread_id;
		user_t* args = malloc(1 * sizeof(user_t));

		args->user_id = i;
		args->priority = i % 2 ? 'N' : 'P';
		

		if (pthread_create(&thread_id, NULL, thread_usuario, (void*)args)) {
			perror("Error on creating a new thread\n");
			free(args);
		} else {
			threads[count] = thread_id;
			count++;
		}
	}

	for (int i = 0; i < count; i++) {
		// Espero a que terminen todos los hilos
		if (pthread_join(threads[i], NULL)) {
			perror("Error on waiting for a thread\n");
		}
	}

	printf("Se han acabado todos los hilos\n");

	free(threads);

	return 0;
}