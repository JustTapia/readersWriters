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
}shm_content;

pthread_mutex_t    *mlector;
pthread_mutexattr_t matr;

pthread_mutex_t    *mptr; 

int messageSize;
int readcount = 0;
int cant_lineas;
int durmiendo;
int leyendo;
void *archivo;

void *leer(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	int i = 0;

	while(1){
		pthread_mutex_lock(mlector);
		readcount++;
		if(readcount==1){
			pthread_mutex_lock(mptr);
		}
		pthread_mutex_unlock(mlector);	

		message *pMensaje;
		while(i<cant_lineas){
			pMensaje = (archivo+(i*messageSize));
			if(pMensaje->pid!=0) break;
			i++;
		}
		if(i!=cant_lineas){
			sleep(leyendo);
			printf(" PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
			fflush(stdout);	
		}
		pthread_mutex_lock(mlector);
		readcount--;
		if(readcount==0){
			pthread_mutex_unlock(mptr);
		}
		pthread_mutex_unlock(mlector);

		sleep(durmiendo);

		i++;
		if(i==cant_lineas){
			i=0;
		}
	}

}

int main(){
	int shmid, shmidMutex;
	key_t key, keyMutex;
	char *shm, *s;
	int nLectores = 0;
	int tiempoDurmiendo = 0;
	int tiempoLeyendo = 0;

	messageSize = sizeof(message);

	printf("Digite la cantidad de lectores");
    scanf("%d", &nLectores);

    printf("Digite el tiempo que duraran escribiendo");
    scanf("%d", &tiempoLeyendo);
    leyendo = tiempoLeyendo;

    printf("Digite el tiempo que dormiran");
    scanf("%d", &tiempoDurmiendo);
    durmiendo = tiempoDurmiendo;

	keyMutex = 5678; 

	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	struct shm_content *pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex);
	cant_lineas = pMutex->cant_lineas;

	int tamano = messageSize*cant_lineas;
	key = 4321; 
	if ((shmid = shmget(key, tamano, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	int rtn;

	mlector =(pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

	if (rtn = pthread_mutexattr_init(&matr))
    {
        fprintf(stderr,"pthreas_mutexattr_init: %s",strerror(rtn)),exit(1);
    }
    if (rtn = pthread_mutexattr_setpshared(&matr,PTHREAD_PROCESS_SHARED))
    {
        fprintf(stderr,"pthread_mutexattr_setpshared %s",strerror(rtn)),exit(1);
    }

    if (rtn = pthread_mutex_init(mlector, &matr))
    {
        fprintf(stderr,"pthread_mutex_init %s",strerror(rtn)), exit(1);
    }

	archivo = shmat(shmid, NULL,0);


	int i = 0;
	pthread_t thread_id[nLectores]; 
	while(i < nLectores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  leer, (void *)pid);
		i++;
	}

	i=0;
	while(i < nLectores){
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