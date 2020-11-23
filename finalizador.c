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
	char pid_w[4];
	char pid_r[4];
	char pid_e[4];

	keyMutex = 5678; 

	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	struct shm_content *pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	

	pid_writer = pMutex->pid_writer;
	pid_reader = pMutex->pid_reader;
	pid_readerEgoista = pMutex->pid_readerEgoista;
	printf("%d\n", pid_writer);
	printf("%d\n", pid_reader);
	printf("%d\n", pid_readerEgoista);
	fflush(stdout);

	/*sprintf(pid_w, "%d", pid_writer);
	sprintf(pid_r, "%d", pid_reader);
	sprintf(pid_e, "%d", pid_readerEgoista);

	char kill_writer[12] = "kill -9 ";
	char kill_reader[12] = "kill -9 ";
	char kill_readerEgoista[12] = "kill -9 ";


	strncat(kill_writer, pid_w, 4);
	printf("%s\n", kill_writer);
	fflush(stdout);
	strncat(kill_reader, pid_r, 4);
	printf("%s\n", kill_reader);
	fflush(stdout);
	strncat(kill_readerEgoista, pid_e, 4);
	printf("%s\n", kill_readerEgoista);
	fflush(stdout);*/

	if(pid_writer!=0){
		//system(kill_writer);
		//kill(pid_writer, SIGTERM);
		kill(pid_writer, SIGKILL);
	}
	
	if(pid_reader!=0){
		//system(kill_reader);
		//kill(pid_reader, SIGTERM);
		kill(pid_reader, SIGKILL);
	}

	if(pid_readerEgoista!=0){
		//system(kill_readerEgoista);
		//kill(pid_readerEgoista, SIGTERM);
		kill(pid_readerEgoista, SIGKILL);
	}


	system("ipcrm -M 5678");
	system("ipcrm -M 4321");
    system("ipcrm -M 8745");
    system("ipcrm -M 4587");
    system("ipcrm -M 7854");
    
}