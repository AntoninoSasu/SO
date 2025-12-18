#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#define CAPACITY 2
#define LIMPIAR 3
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")



pthread_mutex_t m;
pthread_cond_t vip_queue, normal_queue;
int total_vip_queue = 0, total_normal_queue = 0;

int total_disco = 0;
int ticket_vip = 0, ticket_normal = 0;
int turno_vip = 0, turno_normal = 0;


pthread_cond_t servicio_limpieza;
int tres_clientes = 0;
int clean_pending = 0; //1: dentro de enter_cleaning()

typedef struct{
	int id;
	int vip;
}tCliente;

void enter_normal_client(int id)
{
	int turno;
	pthread_mutex_lock(&m);
	turno = ticket_normal++;
	
	printf("LLega el cliente normal %d\n", id);
	while(total_disco == CAPACITY || clean_pending == 1 || tres_clientes == 3|| total_vip_queue > 0 || turno != turno_normal){
		total_normal_queue++;
		pthread_cond_wait(&normal_queue, &m);
		total_normal_queue--;
	}

	//Entra a la discoteca
	tres_clientes++;
	total_disco++;
	turno_normal++;
	if (total_normal_queue > 0)
		pthread_cond_broadcast(&normal_queue);
	pthread_mutex_unlock(&m);
}

void enter_vip_client(int id)
{
	int turno;
	pthread_mutex_lock(&m);
	turno = ticket_vip++;
	
	printf("LLega el cliente vip %d\n", id);
	while(total_disco == CAPACITY || clean_pending == 1 || tres_clientes == 3|| turno != turno_vip){
		total_vip_queue++;
		pthread_cond_wait(&vip_queue, &m);
		total_vip_queue--;
	}

	//Entra a la discoteca
	tres_clientes++;
	total_disco++;
	turno_vip++;
	if (total_vip_queue > 0)
		pthread_cond_broadcast(&vip_queue);
	else if (total_normal_queue > 0)
		pthread_cond_broadcast(&normal_queue);
	pthread_mutex_unlock(&m);
}

void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
	pthread_mutex_lock(&m);
	total_disco--;
	
	
	printf("Sale de la discoteca el cliente (%s) %d\n",VIPSTR(isvip), id);


	if(total_disco == 0 && tres_clientes == LIMPIAR){
		clean_pending = 1;
		pthread_cond_signal(&servicio_limpieza);
	}
	else if(total_vip_queue > 0)
		pthread_cond_broadcast(&vip_queue);
	else if(total_normal_queue > 0)
		pthread_cond_broadcast(&normal_queue);
	
	pthread_mutex_unlock(&m);
}

void *client(void *arg)
{
	tCliente *cliente = (tCliente*) arg;

	if(cliente->vip == 0) //cliente normal
		enter_normal_client(cliente->id);
	else
		enter_vip_client(cliente->id);

	dance(cliente->id, cliente->vip);

	disco_exit(cliente->id, cliente->vip);

	free(cliente);
}

void enter_cleaning(){
	pthread_mutex_lock(&m);
	while(clean_pending == 0){
		pthread_cond_wait(&servicio_limpieza, &m);
	}
	pthread_mutex_unlock(&m);
}

void exit_cleaning(){
	pthread_mutex_lock(&m);
	tres_clientes = 0;
	clean_pending = 0;

	if(total_vip_queue > 0)
		pthread_cond_broadcast(&vip_queue);
	else if(total_normal_queue > 0)
		pthread_cond_broadcast(&normal_queue);

	pthread_mutex_unlock(&m);
}


void* cleaning_thread(void*arg){
	while(1){
		enter_cleaning();
		//Limpiar la discoteca
		printf("Cleaning...\n");
		sleep(2);
		printf("Done!\n");
		exit_cleaning();
	}
}


int main(int argc, char *argv[])
{
	if(argc < 2){
		perror("Faltan argumentos en la linea de comandos\n");
		exit(1);
	}

	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&vip_queue, NULL);
	pthread_cond_init(&normal_queue, NULL);
	pthread_cond_init(&servicio_limpieza, NULL);

	pthread_t hilo_servicio_limpieza;
	pthread_create(&hilo_servicio_limpieza, NULL, cleaning_thread, NULL);


	//Lectura del fichero y creación de los hilos
	FILE* file;
	file = fopen(argv[1], "r");
	int N;
	fscanf(file, "%d", &N);
	pthread_t *hilos = (pthread_t*) malloc(N*sizeof(pthread_t));
	for(int i = 0; i < N; i++){
		tCliente *cliente = (tCliente*) malloc(sizeof(tCliente));
		cliente->id = i;
		fscanf(file, "%d", &cliente->vip);
		pthread_create(&hilos[i], NULL, client, cliente);
	}

	//Barrera, espero a que todos los hilos terminen 
	for(int i = 0; i < N; i++)
		pthread_join(hilos[i], NULL);
	

	pthread_cancel(hilo_servicio_limpieza);

	fclose(file);

	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&vip_queue);
	pthread_cond_destroy(&normal_queue);
	pthread_cond_destroy(&servicio_limpieza);
	
	free(hilos);

	return 0;
}
