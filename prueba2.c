#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <pthread.h> 

struct shm_content
{
      pthread_mutex_t   mutex;
};

pthread_mutex_t    *mptr; //Mutex Pointer
pthread_mutexattr_t matr; //Mutex Attribute

int   shared_mem_id;      //shared memory Id
int   *mp_shared_mem_ptr; //shared memory ptr -- pointing to mutex

int main(){
	int shmid;
	key_t key;
	char *shm, *s;

	key = 5678;

	// Locate the segment.
	if ((shmid = shmget(key, 5*sizeof(char), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	struct shm_content *pcontent = (struct shm_content*) shmat(shmid, 0, 0);

	mptr = &(pcontent->mutex);

        // Lock mutex and then wait for signal to relase mutex
    printf("child mutex lock \n");
    pthread_mutex_lock( mptr );
    printf("child mutex locked\n");

    int i = 0; // :)

    //busy wait
    while (i<3)
    {
        printf("Busy Wait!!! I AM PROCESS 2\n");
        //in the second process will change this line to :
        //printf("Busy Wait!!! I AM PROCESS 2\n");
        sleep(2);
        i++;
    }

    printf("child mutex unlock\n");
    pthread_mutex_unlock( mptr );
    printf("child mutex unlocked\n");
}