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
}shm_content;

pthread_mutex_t    *mptr; //Mutex Pointer
pthread_mutexattr_t matr; //Mutex Attribute

int   shared_mem_id;      //shared memory Id
int   *mp_shared_mem_ptr;


int main(){
	int  rtn;
	char c;
	int nLineas = 0;
	
	int shmidMutex, shmid;
	key_t keyMutex, key;
	
	keyMutex = 5678; 
	system("ipcrm -M 5678");
	// Create the segment.
	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	// Now we attach the segment to our data space.
	shm_content *pMutex = (shm_content*) shmat(shmidMutex, 0, 0);

	mptr = &(pMutex->mutex);

    // Setup Mutex
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
    printf("Digite la cantidad de lineas del archivo");
    scanf("%d", &nLineas);
    int tamano = messageSize*nLineas;
    printf("%d",tamano);
    fflush(stdout);
    key = 4321; 
    system("ipcrm -M 4321");
	// Create the segment.
	if ((shmid = shmget(key, tamano, IPC_CREAT|0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	struct shmid_ds shmbuffer; 

	
	int i = 0;
	void *pInicio = shmat(shmid, NULL,0);
	if(pInicio==-1){ 
		perror("shmat");
		exit(1);}
	while(i < nLineas){
		printf("%d\n",i);
		message *pValor = (pInicio+(i*messageSize));
		pValor->pid = i;	
		i++;
	}

	i = 0;	
	while(i < nLineas){
		message *pValor = (pInicio+(i*messageSize));
		printf("%d\n", pValor->pid);
		i++;
	}

}