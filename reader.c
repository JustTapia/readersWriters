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

typedef struct shm_content//Esta es la estructura principal de control
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

pthread_mutex_t    *mArchivo;
pthread_mutexattr_t matrArchivo;

pthread_mutex_t    *mptr; 

int messageSize;
int readcount = 0;//Para permitir que varios reders pasen y vigilar cuando terminan todos
int cant_lineas;
int durmiendo;
int leyendo;
int *estados;
void *archivo;
struct shm_content *pMutex;
FILE *fptr;
char *cwd;

void *leer(void *vargp){

	int *pid = (int *)vargp;
	message mensaje;
	int i = 0;

	while(1){
		estados[*(pid)-1] = 0;//Estado Bloqueado
		pthread_mutex_lock(mlector);//Mutex para la variable de control

		readcount++;
		if(readcount==1){//Si entra el primer reader
			pthread_mutex_lock(mptr);//Mutex universal
			pMutex->contador_egoista = 0;
		}
		pthread_mutex_unlock(mlector);
		estados[*(pid)-1] = 1;//Estado leyendo

		message *pMensaje;
		while(i<cant_lineas){//Busca el siguiente mensaje a leer
			pMensaje = (archivo+(i*messageSize));
			if(pMensaje->pid!=0) break;
			i++;
		}
		if(i!=cant_lineas){
			sleep(leyendo);
			time_t hora = time(0);
			printf("Proceso Reader\nPID del proceso: %d\nHora en que leyo: %sMensaje que leyo:\nPID: %d\nFecha y Hora: %sLinea: %d\n\n", 
				*pid,ctime(&hora),pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
			fflush(stdout);//Lee el mensaje
			
			pthread_mutex_lock(mArchivo);//Escribe en el archivo

			fptr = fopen(cwd,"a");
			fprintf(fptr, "Proceso Reader\n");
			fprintf(fptr, "PID del proceso: %d\n", *pid);
			fprintf(fptr, "Hora en que leyo: %s", ctime(&(hora)));
			fprintf(fptr, "Mensaje que leyo:\n");
			fprintf(fptr, "PID: %d\nFecha y Hora: %sLinea: %d\n\n", pMensaje->pid,asctime(gmtime(&(pMensaje->fechaHora))),pMensaje->linea);
			fprintf(fptr, "\n");
   			fclose(fptr);

			pthread_mutex_unlock(mArchivo);//Libera el archivo

		}
		pthread_mutex_lock(mlector);//Mutex para la variable de control
		readcount--;
		if(readcount==0){
			pthread_mutex_unlock(mptr);//Si sale el ultimo reader
		}
		pthread_mutex_unlock(mlector);
		estados[*(pid)-1] = 2;//estado durmiendo
		sleep(durmiendo);

		i++;
		if(i==cant_lineas){
			i=0;
		}//Reinicia las lineas si llegó al final del archivo
	}

}

int main(){
	int shmid, shmidMutex, shmidEstado;//Para el id de shmget
	key_t key, keyMutex, keyEstado;//La llave para crear la memoria compartida
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

    //Get mutex del archivo---------------------------------------------------
	keyMutex = 5678; 
	if ((shmidMutex = shmget(keyMutex, sizeof(shm_content), 0666)) < 0) {
		perror("shmget");
		exit(1);
	}
	pMutex = (struct shm_content*) shmat(shmidMutex, 0, 0);
	mptr = &(pMutex->mutex);//Get mutex
	cant_lineas = pMutex->cant_lineas;//Get cantidad de lineas en el archivo
	pMutex->cant_readers = nLectores;//Guarda la cantidad de lectores
	pMutex->pid_reader = getpid();//Guarda el pid del proceso principal de los readers
	cwd = pMutex->cwd;//Obtiene el path de la bitácora

	//Get archivo-------------------------------------------------------------
	int tamano = messageSize*cant_lineas;
	key = 4321; 
	if ((shmid = shmget(key, tamano, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	//Crea mutex de los readers--------------------------------------------------
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

    //Crea mutex para escribir en la bitacora------------------------------------------------------
    int rtnArchivo;

	mArchivo =(pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

	if (rtnArchivo = pthread_mutexattr_init(&matrArchivo))
    {
        fprintf(stderr,"pthreas_mutexattr_init: %s",strerror(rtnArchivo)),exit(1);
    }
    if (rtnArchivo = pthread_mutexattr_setpshared(&matrArchivo,PTHREAD_PROCESS_SHARED))
    {
        fprintf(stderr,"pthread_mutexattr_setpshared %s",strerror(rtnArchivo)),exit(1);
    }

    if (rtnArchivo = pthread_mutex_init(mArchivo, &matrArchivo))
    {
        fprintf(stderr,"pthread_mutex_init %s",strerror(rtnArchivo)), exit(1);
    }

	archivo = shmat(shmid, NULL,0);

	//Array de estados-----------------------------------------------------------------------------
	keyEstado=4587;
	int arrayEstados[nLectores];
	if ((shmidEstado = shmget(keyEstado, sizeof(arrayEstados), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	estados = shmat(shmidEstado, NULL,0);


	//Inicio de Threads--------------------------------------------------------------------------------
	int i = 0;
	pthread_t thread_id[nLectores]; 
	while(i < nLectores){
		int *pid = (int*) malloc(sizeof(int));
		*pid = i+1;
		pthread_create(&thread_id[i], NULL,  leer, (void *)pid);
		i++;
	}

	i=0;
	while(i < nLectores){
		pthread_join(thread_id[i], NULL);
		i++;
	}

}