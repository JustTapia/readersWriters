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
		estados[*(pid)-1]=0; //estado bloqueado
		pthread_mutex_lock(mptr);
		*cont_egoista= *cont_egoista+1; //aumentar contador de egoistas consecutivos

		if(*cont_egoista <= 3){	
			printf("Contador de procesos egoístas seguidos: %d\n", *cont_egoista);
			estados[*(pid)-1]=1; //estado ejecutando

			int linea = rand() % cant_lineas; 
			message *pMensaje = (archivo+(linea*messageSize));
			if(pMensaje->pid!=0){
				sleep(leyendo);
				time_t hora = time(0);

				//Imprimir accion en consola
				printf("Proceso Reader Egoista\n");
				printf("PID del proceso: %d\n", *pid);
				printf("Hora en que leyo: %s", ctime(&(hora)));
				printf("Mensaje que leyo:\n");
				printf("PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
				fflush(stdout);


				//Escribir accion en bitacora
				fptr = fopen(cwd,"a");
				fprintf(fptr,"%s", "Proceso Reader Egoista\n");
				fprintf(fptr,"PID del proceso: %d\n", *pid);
				fprintf(fptr,"Hora en que leyo: %s", ctime(&(hora)));
				fprintf(fptr,"Mensaje que leyo:\n");
				fprintf(fptr,"PID: %d\nFecha y Hora: %sLinea: %d\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
				fprintf(fptr, "\n");
   				fclose(fptr);

   				//Borrar linea
				pMensaje->pid = 0;
				pMensaje->fechaHora = 0;
				pMensaje->linea = 0;
			}else{
				//Linea al azar vacia--------------

				//Imprimir accion en consola
				printf("Proceso Reader Egoista\n");
				printf("PID del proceso: %d\n", *pid);
				printf("Turno perdido, linea al azar vacía :(\n");
				printf("\n");

				//Escribir accion en bitacora
				fptr = fopen(cwd,"a");
				fprintf(fptr,"%s", "Proceso Reader Egoista\n");
				fprintf(fptr,"PID del proceso: %d\n", *pid);
				fprintf(fptr, "Turno perdido, linea al azar vacía :(\n");
				fprintf(fptr, "\n");
   				fclose(fptr);

			}
		}
		
		pthread_mutex_unlock(mptr);

		estados[*(pid)-1]=2; //estado durmiendo
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

    //Get Struct del Mutex----------------------------------------------------------
	keyMutex = 5678; 
	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	struct shm_content *pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex); //get mutex
	cant_lineas = pMutex->cant_lineas; //get cantidad de lineas del archivo
	cont_egoista = &(pMutex->contador_egoista); //get contador de readers egoistas
	pMutex->cant_readersEgoista = nLectores; //asignar cantidad de readers egoistas
	pMutex->pid_readerEgoista = getpid(); //asignar el pid del proceso principal de los egoistas
	cwd = pMutex->cwd; //get path de la bitacora


	//Get Archivo------------------------------------------------------------------
	int tamano = messageSize*cant_lineas;
	key = 4321; 
	if ((shmid = shmget(key, tamano, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}


	archivo = shmat(shmid, NULL,0); //primera linea del archivo

	//Array de Estados-------------------------------------------------------------
	keyEstado=7854;
	int arrayEstados[nLectores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0);


	//Llamada a threads egoistas-------------------------------------------------
	int i = 0;
	pthread_t thread_id[nLectores]; 
	while(i < nLectores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  leerEgoista, (void *)pid);
		i++;
	}

	i=0;
	while(i < nLectores){ //esperar a que terminen los threads
		pthread_join(thread_id[i], NULL);
		i++;
	}


}


