/*
	Contreras Avilès Citali Anahì
	Lòpez Aguirre Gabriel
	Reyes Rodriguez Cinthya Ivonne
	Torres Hinojosa Jose Alfredo
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
/* sockets */
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>	
#include <sys/types.h>
#include <sys/socket.h>

#define CHANCE 6
#define MAX 0x100
#define BUFFER 1024
#define CANTIDAD(x) (sizeof((x))/sizeof(char*)-1)

const char *palabra[]={"ABEJA","AGUILA","BALLENA","BESUGO","BUHO","BUITRE","BURRO"
	"CABALLO","CAMALEON","CAMELLO","CANGURO","CANARIO","CANGREJO","CARACOL",
	"CASTOR","CEBRA","CERDO","CHACAL","CHIMPANCE","CHINCHILLA","CIEMPIES",
	"CIERVO","CISNE","COBAYA","COCODRILO","COLIBRI","CHANGO","COMADREJA","CONEJO",
	"CORMORAN","CONDOR","COTORRA","COYOTE","DELFIN","DRAGON","ESCORPION",
	"FLAMENCO","FOCA","GALLINA","GANSO","GATO","GAVILAN","HALCON","HAMSTER",
	"HIENA","HIPOPOTAMO","HORMIGA","HURON","IGUANA","IMPALA","JABALI","JIRAFA",
	"JAGUAR","KIWI","KOALA","LAGARTO","LANGOSTA","LECHUZA","LEMUR","LEON",
	"LEOPARDO","LIBELULA","LIEBRE","LINCE","OSOPARDO"};

char inttochar(int x) {
	return x+'0';
}

const char *elegida;

void *Palabra(){ 
   	
		srand(time(NULL));
		int aleatoria;
		aleatoria = rand()%CANTIDAD(palabra);

		elegida = palabra[aleatoria];
		printf("%s | %ld\n", elegida, strlen(elegida));
		
}

void codigo_proceso_ahorcado (int id) {

	struct sockaddr_in servidor_direccion; 
    struct sockaddr_in cliente_direccion;
    int tam_struct_servidor;
    int tam_struct_cliente;
    int puerto;
    
	int socket_servidor, socket_cliente;
	int socket_bind;
	int socket_listen;
	char cadenaCliente[BUFFER];
    char cadenaServidor[BUFFER];
	
	socket_servidor = socket(AF_INET, SOCK_STREAM, 0); // creacion del socket
	if(socket_servidor == -1) {
        printf("No se puede crear el socket \n");
        exit(-1);
    }
    printf("Ingrese el puerto de escucha:\n");
    scanf("%d", &puerto);
	
	servidor_direccion.sin_addr.s_addr = inet_addr("127.0.0.1");
	servidor_direccion.sin_family = AF_INET;
    servidor_direccion.sin_port = puerto;
    tam_struct_servidor = sizeof(servidor_direccion);
	
	socket_bind = bind(socket_servidor, (struct sockaddr *)&servidor_direccion, tam_struct_servidor);
	if(socket_bind == -1) {
		printf("No se puede crear el bind\n");
		exit(-1);
	}
		
	int backlog = 1;
	socket_listen = listen(socket_servidor, backlog); 
	if(socket_listen == -1) {
		printf("No se puede crear el listen\n");
		exit(-1);
	}
	printf("Servidor en espera...\n");
	
	while(1) {
		socket_cliente = accept(socket_servidor, (struct sockaddr *)&cliente_direccion, &tam_struct_servidor);
		if(socket_cliente == -1) {
			printf("No se puede crear el accept \n");
			exit(-1);
		}
		printf("\t-- CLIENTE CONECTADO --\n");
		pthread_t hilo1;
		pthread_create(&hilo1, NULL, Palabra, NULL);
		pthread_join(hilo1, NULL);	
	
		int oportunidades = CHANCE;		
		int N = strlen(elegida);
		int mask[N];
		char aux[BUFFER];
		for (int i=0; i < N; ++i) {
		    mask[i] = 0;
		}
		char copia[BUFFER];
		strcpy(copia, elegida);
		int cont = 0;
		int gameover = 0;
		char opor[BUFFER] = ""; 
		while(strcmp(cadenaServidor, "SALIR") != 0 && gameover == 0) {
			strcpy(opor, "NO");
			int flag = 0;              
			send(socket_cliente, "\nLa palabra es: ", BUFFER, 0);	
			for(int j=0; j < N; ++j) {
				if (mask[j] == 1) {
				    aux[j] = copia[j];
				}
				else {
				    aux[j] = '_';
				}
			}
			aux[N] = '\0';
			send(socket_cliente, aux, BUFFER, 0);	
		
			char guess[2];
			send(socket_cliente, "\nIngrese una ", BUFFER, 0);	
			recv(socket_cliente, guess, BUFFER, 0);
				
			for(int k=0; k < N; ++k) {
				if (copia[k] == guess[0]) {
				    mask[k] = 1;
				    flag = 1;
				    strcpy(opor, "SI");
				} 
			}
			send(socket_cliente, opor, BUFFER, 0);
			if(flag == 0) {												
				oportunidades--;
				char c = inttochar(oportunidades);
				char o[50] = "Te quedan ";
				int x = strlen(o);
				o[strlen(o)] = c;
				strcat(o, " oportunidades.\n");
				send(socket_cliente, o, BUFFER, 0);
				if(oportunidades == 0) {
				   	send(socket_cliente, "BREAK", BUFFER, 0);
				    break;
				}				        
			}

			for(int m = 0; m < N; ++m) {
				cont += mask[m];
				if (mask[m] == 0) {            	
				    cont = 0;
				    break;
				} else if(cont == N) {
				   	gameover = 1;
				    send(socket_cliente, "BREAK", BUFFER, 0);
				}
			}
		}		
		if(oportunidades == 0) {        
			char p[50] = "\nPerdiste! La palabra es ";
			copia[N] = '\n';
			copia[N+1] = '\0';
			strcat(p, copia);
			send(socket_cliente, p, BUFFER, 0);
		} else {
			char g[50] = "\nGanaste! La palabra es ";
			copia[N] = '\n';
			copia[N+1] = '\0';
			strcat(g, copia);
			send(socket_cliente, g, BUFFER, 0);
		}
		close(socket_cliente);
	} 	
	close(socket_servidor);
	exit(id);
}



int main() {
	int pids;
    int pid;
    int salida;
    int cont;

    pid = fork();
    pids = pid;
          
    if(pid == -1) {
           perror("No se puede crear el proceso \n");
           exit(-1);
       } else if(pid == 0) { 
           codigo_proceso_ahorcado(pids);  
       } 
 
     pid = wait(&salida);
     printf("Proceso %d: con id = %x,terminado\n", pid, salida >>8);

}
			
