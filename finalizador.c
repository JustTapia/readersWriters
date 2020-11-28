#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <pthread.h> 
#include <time.h>
#include <limits.h>
#include <signal.h>

typedef struct shm_content
{
      pthread_mutex_t   mutex;
      int cant_lineas;
      int contador_egoista;
      int cant_writers;
      int cant_readers;
      int cant_readersEgoista;
      int pid_writer;
      int pid_reader;
      int pid_readerEgoista;
      char cwd[PATH_MAX];
}shm_content;

pthread_mutex_t    *mptr; 


int main(){
	int shmid, shmidMutex, shmidEstado;
	pid_t pid_writer, pid_reader, pid_readerEgoista;
	key_t key, keyMutex, keyEstado;

	keyMutex = 5678; 

	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	struct shm_content *pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0); //memoria compartida del mutex
	

	pid_writer = pMutex->pid_writer; //pid del proceso de los writers
	pid_reader = pMutex->pid_reader; //pid del proceso de los readers
	pid_readerEgoista = pMutex->pid_readerEgoista; //pid del proceso de los reader egoistas


	if(pid_writer!=0){ //si se ejecutaron los writers
		//kill(pid_writer, SIGTERM);
		kill(pid_writer, SIGKILL);
	}
	
	if(pid_reader!=0){ //si se ejecutaron los readers
		//kill(pid_reader, SIGTERM);
		kill(pid_reader, SIGKILL);
	}

	if(pid_readerEgoista!=0){ //si se ejecutaron los egoistas
		//kill(pid_readerEgoista, SIGTERM);
		kill(pid_readerEgoista, SIGKILL);
	}
	printf("Procesos Terminados\n");

	system("ipcrm -M 5678"); //eliminar memoria compartida mutex
	system("ipcrm -M 4321"); //eliminar memoria compartida archivo
    system("ipcrm -M 8745"); //eliminar memoria compartida estados de writers
    system("ipcrm -M 4587"); //eliminar memoria compartida estados de readers
    system("ipcrm -M 7854"); //eliminar memoria compartida estados de egoistas
    printf("Archivo Eliminado\n");
    
}