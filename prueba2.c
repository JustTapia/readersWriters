#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>


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

	//  Now we attach the segment to our data space.
	if ((shm = shmat(shmid, NULL, 0)) == (char*) -1) {
		perror("shmat");
		exit(1);
	}
	printf("%s",shm);
}