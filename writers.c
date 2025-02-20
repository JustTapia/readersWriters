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
int cant_lineas;
int durmiendo;
int escribiendo;
void *archivo;
int *estados;
FILE *fptr;

int messageSize;
struct shm_content *pMutex;
char *cwd;


void *escribir(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	
	while(1){

		estados[*(pid)-1] = 0; //estado bloqueado
		pthread_mutex_lock(mptr);
		estados[*(pid)-1] = 1; //estado ejecutando
		pMutex->contador_egoista = 0; //reiniciar contador egoistas consecutivos

		int i = 0;
		message *pMensaje;
		while(i<cant_lineas){ //busca linea vacia para escribir
			pMensaje = (archivo+(i*messageSize));
			if(pMensaje->pid==0) break;
			i++;
		}
		if(i!=cant_lineas){
			sleep(escribiendo);
			
			pMensaje->pid = *pid;
			pMensaje->fechaHora = time(0);
			pMensaje->linea = i;

			//Imprimir accion en consola
			printf("Proceso Writer\n");
			printf("Mensaje que escribio:\n");
			printf("PID: %d\n", pMensaje->pid);
			printf("Fecha y Hora: %s", ctime(&(pMensaje->fechaHora)));
			printf("Linea: %d\n", pMensaje->linea);
			printf("\n");
			fflush(stdout);

			//Escribir accion en bitacora
			fptr = fopen(cwd,"a");
			fprintf(fptr,"%s", "Proceso Writer\n");
			fprintf(fptr, "Mensaje que escribio:\n");
			fprintf(fptr, "PID: %d\n", pMensaje->pid);
			fprintf(fptr,"Fecha y Hora: %s", ctime(&(pMensaje->fechaHora)));
			fprintf(fptr, "Linea: %d\n", pMensaje->linea);
			fprintf(fptr, "\n");
   			fclose(fptr);

			pthread_mutex_unlock(mptr);

			estados[*(pid)-1] = 2; //estado durmiendo
			sleep(durmiendo);
		}else{
			pthread_mutex_unlock(mptr);

			estados[*(pid)-1] = 2; //estado durmiendo
			sleep(durmiendo);
		} 
	}

}

int main(){
	int shmid, shmidMutex, shmidEstado;
	key_t key, keyMutex, keyEstado;
	char *shm, *s;
	int nEscritores = 0;
	int tiempoDurmiendo = 0;
	int tiempoEscribiendo = 0;

	messageSize = sizeof(message);

	printf("Digite la cantidad de escritores: ");
    scanf("%d", &nEscritores);

    printf("Digite el tiempo que duraran escribiendo: ");
    scanf("%d", &tiempoEscribiendo);
    escribiendo = tiempoEscribiendo;

    printf("Digite el tiempo que dormiran: ");
    scanf("%d", &tiempoDurmiendo);
    durmiendo = tiempoDurmiendo;


    //Get Struct del Mutex----------------------------------------------------------
	keyMutex = 5678; 
	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex); //get mutex
	cant_lineas = pMutex->cant_lineas; //get cantidad de lineas del archivo
	pMutex->cant_writers = nEscritores; //asignar cantidad de writers
	pMutex->pid_writer = getpid(); //asignar pid del proceso principal de los writers
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
	keyEstado=8745;
	int arrayEstados[nEscritores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0); 

	//Llamada a threads escritores-------------------------------------------------
	int i = 0;
	pthread_t thread_id[nEscritores];
	while(i < nEscritores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  escribir, (void *)pid);
		i++;
	}

	i=0;
	while(i < nEscritores){ //esperar a que terminen los threads
		pthread_join(thread_id[i], NULL);
		i++;
	}

}