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

pthread_mutex_t    *mptr; //Mutex Pointer
pthread_mutexattr_t matr; //Mutex Attribute


int main(){
	int  rtn;
	char c;
	int nLineas = 0;
	
	int shmidMutex, shmid;
	key_t keyMutex, key;

	printf("Digite la cantidad de lineas del archivo: ");
    scanf("%d", &nLineas);

    //Memoria compartida del mutex------------------------------------------
	keyMutex = 5678; 
	// Crear la estructura de control.
	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}


	shm_content *pMutex = (shm_content*) shmat(shmidMutex, 0, 0);

	//Crea el mutex---------------------------------------------------------
	mptr = &(pMutex->mutex);

    if (rtn = pthread_mutexattr_init(&matr))
    {
        fprintf(stderr,"pthreas_mutexattr_init: %s",strerror(rtn)),exit(1);
    }
    if (rtn = pthread_mutexattr_setpshared(&matr,PTHREAD_PROCESS_SHARED))
    {
        fprintf(stderr,"pthread_mutexattr_setpshared %s",strerror(rtn)),exit(1);
    }
    if (rtn = pthread_mutex_init(mptr, &matr))
    {
        fprintf(stderr,"pthread_mutex_init %s",strerror(rtn)), exit(1);
    }

    int messageSize = sizeof(message);
    
    int tamano = messageSize*nLineas;
    pMutex->cant_lineas = nLineas;
    pMutex->contador_egoista = 0;
    getcwd(pMutex->cwd, sizeof(pMutex->cwd));//Obtiene el Path actual

    strcat(pMutex->cwd, "/bitacora.txt");//Le anade el nombre de la bitacora al path actual

    FILE *fptr;
    fptr = fopen(pMutex->cwd,"w");
   	fclose(fptr);//Reinicia el archivo de la bitacora

   	//Memoria Compartida del archivo------------------------------------------
    key = 4321; 
	// Crea el archivo
	if ((shmid = shmget(key, tamano, IPC_CREAT|0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	
	void *pInicio = shmat(shmid, NULL,0);
	if(pInicio== (void *)-1){ 
		perror("shmat");
		exit(1);
	}
	printf("Ambiente inicializado\n");

}
