#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <pthread.h> 
#include <time.h>

typedef struct message
{
      int pid;
      time_t fechaHora;
      int linea;
} message;

typedef struct shm_content
{
      pthread_mutex_t   mutex;
      int cant_lineas;
      int contador_egoista;
      int cant_writers;
      int cant_readers;
      int cant_readersEgoista;
}shm_content;

pthread_mutex_t    *mptr; 
int cant_lineas;
int durmiendo;
int escribiendo;
void *archivo;
int *estados;

int messageSize;
struct shm_content *pMutex;


void *escribir(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	while(1){
		estados[*(pid)-1] = 0;
		pthread_mutex_lock(mptr);
		estados[*(pid)-1] = 1;
		pMutex->contador_egoista = 0;

		int i = 0;
		message *pMensaje;
		while(i<cant_lineas){
			pMensaje = (archivo+(i*messageSize));
			if(pMensaje->pid==0) break;
			i++;
		}
		if(i!=cant_lineas){
			sleep(escribiendo);
			pMensaje->pid = *pid;
			pMensaje->fechaHora = time(0);
			pMensaje->linea = i;
			printf(" PID: %d\n", pMensaje->pid);
			printf("\n");
			fflush(stdout);
			pthread_mutex_unlock(mptr);
			estados[*(pid)-1] = 2;
			sleep(durmiendo);
		}else{
			pthread_mutex_unlock(mptr);
			sleep(durmiendo);
		} 
	}

}

int main(){
	int shmid, shmidMutex, shmidEstado;
	key_t key, keyMutex, keyEstado;
	char *shm, *s;
	int nEscritores = 0;
	int tiempoDurmiendo = 0;
	int tiempoEscribiendo = 0;

	messageSize = sizeof(message);

	printf("Digite la cantidad de escritores");
    scanf("%d", &nEscritores);

    printf("Digite el tiempo que duraran escribiendo");
    scanf("%d", &tiempoEscribiendo);
    escribiendo = tiempoEscribiendo;

    printf("Digite el tiempo que dormiran");
    scanf("%d", &tiempoDurmiendo);
    durmiendo = tiempoDurmiendo;

	keyMutex = 5678; 

	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex);
	cant_lineas = pMutex->cant_lineas;
	pMutex->cant_writers = nEscritores;

	int tamano = messageSize*cant_lineas;
	key = 4321; 
	if ((shmid = shmget(key, tamano, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	archivo = shmat(shmid, NULL,0);


	//Array de Estados-------------------------------------------------------------
	keyEstado=8745;
	int arrayEstados[nEscritores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0);

	//Llamada a threads escritores-------------------------------------------------
	int i = 0;
	pthread_t thread_id[nEscritores];
	while(i < nEscritores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  escribir, (void *)pid);
		i++;
	}

	



	i=0;
	while(i < nEscritores){
		pthread_join(thread_id[i], NULL);
		i++;
	}

	/*i = 0;
	char MY_TIME[50];

	while(i < cant_lineas){
		message *pValor = (archivo+(i*messageSize));
		printf(" PID: %d\n", pValor->pid);
		printf("Fecha y Hora: %s", asctime(gmtime(&(pValor->fechaHora))));
		printf("Linea: %d\n", pValor->linea);
		printf("\n");
		i++;
	}*/

}