#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <pthread.h> 

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
	int shmid;
	key_t key;
	

	key = 5678; 

	// Create the segment.
	if ((shmid = shmget(key, sizeof(shm_content), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	// Now we attach the segment to our data space.
	struct shm_content *pcontent = (struct shm_content*) shmat(shmid, 0, 0);

	mptr = &(pcontent->mutex);

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

        // Lock mutex and then wait for signal to relase mutex
    printf("child mutex lock \n");
    pthread_mutex_lock( mptr );
    printf("child mutex locked\n");

    int i = 0; // :)

    //busy wait
    while (i<3)
    {
        printf("Busy Wait!!! I AM PROCESS 1\n");
        //in the second process will change this line to :
        //printf("Busy Wait!!! I AM PROCESS 2\n");
        sleep(2);
        i++;
    }

    printf("child mutex unlock\n");
    pthread_mutex_unlock( mptr );
    printf("child mutex unlocked\n");
}