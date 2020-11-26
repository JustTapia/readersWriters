#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <pthread.h> 
#include <time.h>
#include <limits.h>


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
      pid_t pid_writer;
      pid_t pid_reader;
      pid_t pid_readerEgoista;
      char cwd[PATH_MAX];
}shm_content;

pthread_mutex_t    *mlector;
pthread_mutexattr_t matr;

pthread_mutex_t    *mArchivo;
pthread_mutexattr_t matrArchivo;

pthread_mutex_t    *mptr; 

int messageSize;
int readcount = 0;
int cant_lineas;
int durmiendo;
int leyendo;
int *estados;
void *archivo;
struct shm_content *pMutex;
FILE *fptr;
char *cwd;

void *leer(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	int i = 0;

	while(1){
		estados[*(pid)-1] = 0;
		pthread_mutex_lock(mlector);

		readcount++;
		if(readcount==1){
			pthread_mutex_lock(mptr);
			pMutex->contador_egoista = 0;
		}
		pthread_mutex_unlock(mlector);
		estados[*(pid)-1] = 1;

		message *pMensaje;
		while(i<cant_lineas){
			pMensaje = (archivo+(i*messageSize));
			if(pMensaje->pid!=0) break;
			i++;
		}
		if(i!=cant_lineas){
			sleep(leyendo);
			time_t hora = time(0);
			printf("Proceso Reader\nPID del proceso: %d\nHora en que leyo: %sMensaje que leyo:\nPID: %d\nFecha y Hora: %sLinea: %d\n\n", 
				*pid,ctime(&hora),pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
			fflush(stdout);	
			
			pthread_mutex_lock(mArchivo);

			fptr = fopen(cwd,"a");
			fprintf(fptr, "Proceso Reader\n");
			fprintf(fptr, "PID del proceso: %d\n", *pid);
			fprintf(fptr, "Hora en que leyo: %s", ctime(&(hora)));
			fprintf(fptr, "Mensaje que leyo:\n");
			fprintf(fptr, "PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
			fprintf(fptr, "\n");
   			fclose(fptr);

			pthread_mutex_unlock(mArchivo);

		}
		pthread_mutex_lock(mlector);
		readcount--;
		if(readcount==0){
			pthread_mutex_unlock(mptr);
		}
		pthread_mutex_unlock(mlector);
		estados[*(pid)-1] = 2;
		sleep(durmiendo);

		i++;
		if(i==cant_lineas){
			i=0;
		}
	}

}

int main(){
	int shmid, shmidMutex, shmidEstado;
	key_t key, keyMutex, keyEstado;
	char *shm, *s;
	int nLectores = 0;
	int tiempoDurmiendo = 0;
	int tiempoLeyendo = 0;

	messageSize = sizeof(message);

	printf("Digite la cantidad de lectores: ");
    scanf("%d", &nLectores);

    printf("Digite el tiempo que duraran leyendo: ");
    scanf("%d", &tiempoLeyendo);
    leyendo = tiempoLeyendo;

    printf("Digite el tiempo que dormiran: ");
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
	pMutex->cant_readers = nLectores;
	pMutex->pid_reader = getpid();
	cwd = pMutex->cwd;

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

    int rtnArchivo;

	mArchivo =(pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

	if (rtnArchivo = pthread_mutexattr_init(&matrArchivo))
    {
        fprintf(stderr,"pthreas_mutexattr_init: %s",strerror(rtnArchivo)),exit(1);
    }
    if (rtnArchivo = pthread_mutexattr_setpshared(&matrArchivo,PTHREAD_PROCESS_SHARED))
    {
        fprintf(stderr,"pthread_mutexattr_setpshared %s",strerror(rtnArchivo)),exit(1);
    }

    if (rtnArchivo = pthread_mutex_init(mArchivo, &matrArchivo))
    {
        fprintf(stderr,"pthread_mutex_init %s",strerror(rtnArchivo)), exit(1);
    }

	archivo = shmat(shmid, NULL,0);

	//Array de estados-----------------------------------------------------------------------------
	keyEstado=4587;
	int arrayEstados[nLectores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0);


	//Inicio de Threads--------------------------------------------------------------------------------
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