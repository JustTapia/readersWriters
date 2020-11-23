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

int messageSize;
int cant_lineas;
void *archivo;
int nEscritores;
int nLectores;
int nEgoistas;
int *estadoWriters;
int *estadoLectores;
int *estadoEgoistas;
struct shm_content *pMutex;


void leerArchivo(){
	message *pMensaje;
	for(int i; i < cant_lineas;i++){
		pMensaje = (archivo+(i*messageSize));
		if(pMensaje->fechaHora==0){
			printf(" PID: %d\nFecha y Hora: 0\nLinea: %d\n\n", pMensaje->pid,pMensaje->linea);
		}else{
			printf(" PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
		}
		printf("--------------------------------------------------------\n");
	}
	printf("Archivo:\n");
}



void leerEstadoWriters(){
	printf("Estado Writers:\n");
	for(int i = 0; i < nEscritores; i++){
		printf("PID: %d\t",i+1);
		int estado = estadoWriters[i];
		switch (estado){
			case 1:
				printf("Estado: Escribiendo\n");
				break;
			case 2:
				printf("Estado: Durmiendo\n");
				break;
			default:
				printf("Estado: Bloqueado\n");
				break;
		}
		printf("--------------------------------------------------------\n");
	}
}

void leerEstadoLectores(){
	printf("Estado Readers:\n");
	for(int i = 0; i < nLectores; i++){
		printf("PID: %d\t",i+1);
		int estado = estadoLectores[i];
		switch (estado){
			case 1:
				printf("Estado: Leyendo\n");
				break;
			case 2:
				printf("Estado: Durmiendo\n");
				break;
			default:
				printf("Estado: Bloqueado\n");
				break;
		}
		printf("--------------------------------------------------------\n");
	}
}

void leerEstadoEgoistas(){
	printf("Estado Readers Egoistas:\n");
	for(int i = 0; i < nLectores; i++){
		printf("PID: %d\t",i+1);
		int estado = estadoEgoistas[i];
		switch (estado){
			case 1:
				printf("Estado: Leyendo\n");
				break;
			case 2:
				printf("Estado: Durmiendo\n");
				break;
			default:
				printf("Estado: Bloqueado\n");
				break;
		}
		printf("--------------------------------------------------------\n");
	}
}

int main(){
	int shmid, shmidMutex, shmidEstadoWriters, shmidEstadoLectores, shmidEstadoEgoistas;
	key_t key, keyMutex, keyEstadoWriters, keyEstadoLectores, keyEstadoEgoistas;
	char *shm, *s;

	messageSize = sizeof(message);
	keyMutex = 5678; 

	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	cant_lineas = pMutex->cant_lineas;
	nEscritores = pMutex->cant_writers;
	nLectores = pMutex->cant_readers;
	nEgoistas = pMutex->cant_readersEgoista;

	int tamano = messageSize*cant_lineas;
	key = 4321; 
	if ((shmid = shmget(key, tamano, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	archivo = shmat(shmid, NULL,0);


	//Array de Estados Writers-------------------------------------------------------------
	keyEstadoWriters=8745;
	int arrayEstadoWriters[nEscritores];
	if ((shmidEstadoWriters = shmget(keyEstadoWriters, sizeof(arrayEstadoWriters), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	estadoWriters = shmat(shmidEstadoWriters, NULL,0);
	//Array de estados Readers----------------------------------------------------------------
	keyEstadoLectores=4587;
	int arrayEstadoLectores[nLectores];
	if ((shmidEstadoLectores = shmget(keyEstadoLectores, sizeof(arrayEstadoLectores), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	estadoLectores = shmat(shmidEstadoLectores, NULL,0);
	//Array de estados Readers----------------------------------------------------------------
	keyEstadoEgoistas=7854;
	int arrayEstadoEgoistas[nEgoistas];
	if ((shmidEstadoEgoistas = shmget(keyEstadoEgoistas, sizeof(arrayEstadoEgoistas), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	estadoEgoistas = shmat(shmidEstadoEgoistas, NULL,0);
	leerArchivo();
	printf("\n");
	leerEstadoWriters();
	printf("\n");
	leerEstadoLectores();
	printf("\n");
	leerEstadoEgoistas();
}