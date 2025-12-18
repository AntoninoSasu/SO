#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


typedef struct {
		int thread_num;   // Número de hilo
		char priority;    // Prioridad: 'P' (prioritario) o 'N' (no prioritario)
	} thread_args_t;

	void *thread_usuario(void *arg)   //Entrada: Recibe un puntero a thread_args_t.
	{
		
		thread_args_t *args = (thread_args_t *)arg;
		int thread_num = args->thread_num;
		char priority = args->priority;

		free(args);
		pthread_t thread_id = pthread_self();   

		printf("Hilo ID: %lu, Número de hilo: %d, Prioridad: %c\n",
			thread_id, thread_num, priority);


		pthread_exit(NULL);

	}

	int main(int argc, char* argv[])
	{ int num_threads;

		// Validar argumentos
		if (argc != 2) {
			fprintf(stderr, "Uso: %s <número de hilos>\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		// Convertir argumento a entero
		num_threads = atoi(argv[1]);
		if (num_threads <= 0) {
			fprintf(stderr, "El número de hilos debe ser mayor que 0.\n");
			exit(EXIT_FAILURE);
		}

		pthread_t threads[num_threads]; //array con el numero de hilos

		for (int i = 0; i < num_threads; i++) {

			thread_args_t *args = malloc(sizeof(thread_args_t)); //reservamos memoria para cada hilo
			if (args == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			// Inicializar los argumentos del hilo
			args->thread_num = i;
			args->priority = (i % 2 == 0) ? 'P' : 'N'; //si es apr es prioritario si no no lo es

			// Crear el hilo
			if (pthread_create(&threads[i], NULL, thread_usuario, args) != 0) { 
				perror("pthread_create");
				free(args);  // Liberar memoria en caso de error
				exit(EXIT_FAILURE);
			}
		}

		// Esperar a que todos los hilos terminen
		for (int i = 0; i < num_threads; i++) {  
			if (pthread_join(threads[i], NULL) != 0) {
				perror("pthread_join");
				exit(EXIT_FAILURE);
			}
		}

		printf("Todos los hilos han terminado.\n");

		return 0;
	}
