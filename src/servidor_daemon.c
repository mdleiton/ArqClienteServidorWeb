#include <sys/types.h>        
#include <sys/stat.h>
#include <stdio.h>          
#include <stdlib.h>            
#include <stddef.h>         
#include <string.h>            
#include <unistd.h>                  
#include <netdb.h> 
#include <errno.h> 
#include <syslog.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <libudev.h>
#include <mntent.h>
#include "Monitor.h"
#define BUFLEN 1024 
#define BUFFERING 100000
#define QLEN 50 

#ifndef HOST_NAME_MAX 
#define HOST_NAME_MAX 256 
#endif	
int clfd;  
int filefd;
int sockfd;
int fd;

/* lee el archivo del pendrive */
char* leer_archivo(char* direccion, char* nombre_archivo){
	FILE *archivo;
	int caracter;
	char resultado[1000];
	char* texto_final="";
	sprintf(resultado,"%s/%s", direccion,nombre_archivo);

	archivo = fopen(resultado,"r");
	if (archivo == NULL){
            printf("\nError de apertura del archivo. \n\n");
    } else{
        while((caracter = fgetc(archivo)) != EOF) {
			sprintf(texto_final,"%s%c",texto_final,caracter);
		}
        }

        fclose(archivo);
        return texto_final;
}


//Funcion para inicializar el servidor
int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen){
	int err = 0;
	if((fd = socket(addr->sa_family, type, 0)) < 0)
		return -1;
	if(bind(fd, addr, alen) < 0)
		goto errout;
	if(type == SOCK_STREAM || type == SOCK_SEQPACKET){
		if(listen(fd, QLEN) < 0)
			goto errout;
	}
	return fd;
errout:
	err = errno;
	close(fd);
	errno = err;
	return (-1);
}

void escuchandoSolicitudesClientes(){ 
	int  n,puerto = 8888;
	char *host; 
	if (( n = sysconf(_SC_HOST_NAME_MAX)) < 0) n = HOST_NAME_MAX; /* best guess */ 
	if ((host = malloc(n)) == NULL) printf(" malloc error"); 
	if (gethostname( host, n) < 0) 		//Obtenemos nombre del host
		printf(" gethostname error"); 
	//Direccion del servidor
	struct sockaddr_in direccion_servidor;
	memset(&direccion_servidor, 0, sizeof(direccion_servidor));	//ponemos en 0 la estructura direccion_servidor

	//llenamos los campos
	direccion_servidor.sin_family = AF_INET;		//IPv4
	direccion_servidor.sin_port = htons(puerto);		//Convertimos el numero de puerto al endianness de la red
	direccion_servidor.sin_addr.s_addr = inet_addr("127.0.0.1") ;	//Nos vinculamos a la interface localhost o podemos usar INADDR_ANY para ligarnos A TODAS las interfaces

	if( (sockfd = initserver(SOCK_STREAM, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor), 1000)) < 0){	//Hasta 1000 solicitudes en cola 
		printf("existe un proceso ya ejecutanse. eliminar proceso daemonUSB\n");	
	}		

	while(1){
		//Ciclo para enviar y recibir mensajes
		if (( clfd = accept( sockfd, NULL, NULL)) < 0) { 		//Aceptamos una conexion
			close(clfd);
			continue;
		} 
	    char *solicitud = malloc(BUFLEN*sizeof(char *));
		recv(clfd, solicitud, BUFLEN, 0);
	  	//tratamiento tipo de solicitud
	  	if ((strstr(solicitud, "GET") != NULL) && (strstr(solicitud, "listar_dispositivos") != NULL)) {
  			//solicitud : GET - listar_dispositivo
  			struct udev *udeva;
			udeva = udev_new();
			char* lista=enumerar_disp_alm_masivo(udeva);
		    send(clfd,lista,strlen(lista),0);
		    close(clfd);
		}else if((strstr(solicitud, "obtenerdireccion") != NULL) ) {
			char * respx=malloc(BUFLEN*sizeof(char *));
			int z=0,j=0;
			printf("%d\n",strlen(solicitud));
			for(int i=0;i<strlen(solicitud);i++){
				if(solicitud[i]=='-' && i==0){
					i++;
					while(solicitud[i]!='-'){
						printf("%s\n",solicitud[i]);
						respx[j]=solicitud[i];
						i++;
						j++;
					}
					z++;

				}
				
			}
			respx[j] = '\0';
			printf("sdc%svdvdv\n",respx);sdcvdvdv
			//char* upload_datas=malloc(sizeof(char)*(strlen(upload_data)));
  			//strncpy(upload_datas, upload_data+1, strlen( upload_data)-2);
			respx=Dispositivo(solicitud);
			send(clfd,respx,strlen(respx),0);
		    close(clfd);
		}
		close(clfd);
			
		
	}
}

