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

pthread_mutex_t    *mptr; 

int messageSize;
int readcount = 0;
int cant_lineas;
int durmiendo;
int leyendo;
int *cont_egoista;
int *estados;
void *archivo;
FILE *fptr;
char *cwd;


void *leerEgoista(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	int i = 0;

	while(1){
		estados[*(pid)-1]=0;
		pthread_mutex_lock(mptr);
		*cont_egoista++;
		if(*cont_egoista <= 3){
			estados[*(pid)-1]=1;
			int linea = rand() % cant_lineas;
			message *pMensaje = (archivo+(linea*messageSize));

			if(pMensaje->pid!=0){

				sleep(leyendo);
				time_t hora = time(0);
				printf("Proceso Reader Egoista\n");
				printf("PID del proceso: %d\n", *pid);
				printf("Hora en que leyo: %s", ctime(&(hora)));
				printf("Mensaje que leyo:\n");
				printf("PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
				fflush(stdout);

				fptr = fopen(cwd,"a");
				fprintf(fptr,"%s", "Proceso Reader Egoista\n");
				fprintf(fptr,"PID del proceso: %d\n", *pid);
				fprintf(fptr,"Hora en que leyo: %s", ctime(&(hora)));
				fprintf(fptr,"Mensaje que leyo:\n");
				fprintf(fptr,"PID: %d\nFecha y Hora: %sLinea: %d\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
				fprintf(fptr, "\n");
   				fclose(fptr);

				pMensaje->pid = 0;
				pMensaje->fechaHora = 0;
				pMensaje->linea = 0;
			}else{
				printf("Proceso Reader Egoista\n");
				printf("PID del proceso: %d\n", *pid);
				printf("Turno perdido, linea al azar vacía :(\n\n");

				fptr = fopen(cwd,"a");
				fprintf(fptr,"%s", "Proceso Reader Egoista\n");
				fprintf(fptr,"PID del proceso: %d\n", *pid);
				fprintf(fptr, "Turno perdido, linea al azar vacía :(\n");
				fprintf(fptr, "\n");
   				fclose(fptr);

			}

		}

		pthread_mutex_unlock(mptr);
		estados[*(pid)-1]=2;
		sleep(durmiendo);
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
	struct shm_content *pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex);
	cant_lineas = pMutex->cant_lineas;
	cont_egoista = &(pMutex->contador_egoista);
	pMutex->cant_readersEgoista = nLectores;
	pMutex->pid_readerEgoista = getpid();
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



	archivo = shmat(shmid, NULL,0);

	keyEstado=7854;
	int arrayEstados[nLectores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0);


	int i = 0;
	pthread_t thread_id[nLectores]; 
	while(i < nLectores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  leerEgoista, (void *)pid);
		i++;
	}

	i=0;
	while(i < nLectores){
		pthread_join(thread_id[i], NULL);
		i++;
	}

	i = 0;
	char MY_TIME[50];

	/*while(i < cant_lineas){
		message *pValor = (archivo+(i*messageSize));
		printf(" PID: %d\n", pValor->pid);
		printf("Fecha y Hora: %s", asctime(gmtime(&(pValor->fechaHora))));
		printf("Linea: %d\n", pValor->linea);
		printf("\n");
		i++;
	}*/

}


