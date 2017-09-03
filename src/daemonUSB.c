#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>
#include <stdio.h>
#include <mntent.h>
#include "Monitor.h"

int main(void) {
	/*Estructura básica de un demonio:
	    Hacer fork del proceso padre. (terminar proceso padre)
	    Cambiar la máscara de fichero.
	    Abrir los archivos de registros necesarios. ((fichero log para daemon)
	    Crear un identificador de sesión único.
	    Cambiar el directorio de trabajo hacia un sitio más seguro.
	    Cerrar los descriptores de ficheros estandard.
	    Escribir el código del demonio  */
	
	pid_t pid, sid;
  	int logsdaemon;     
          
	pid = fork();
	/* validar fork retorno */
	if (pid < 0) {
		return -1;
	}
	/* finalizar proceso padre */
	if (pid > 0) {
		return -1;
	}

	/*  mascara de ficheros cambiado: acceso de otro usuario a ficheros generados aqui*/
	umask(0);
	      
	/*ficheros de logs del demonio para poder comunicarse con el daemon */        
	logsdaemon = open ("ACTUALIZACION", O_WRONLY | O_CREAT, 0600);
	/* validar apertura de fichero */
	if (logsdaemon == -1) {
		perror ("Error fichero log daemon");
		return -1;
	}
	/* asignar un nuevo pid evitando problemas que se genere un proceso zombie*/
	/* y validar nuevo id para procesos */
	
	sid = setsid();
	
	if (sid < 0) {
		perror("new SID failed");
		
	}

	/* recomendable cambiar wd (medida de seguridad)*/

	if ((chdir("/")) < 0) {
		perror("error al cambiar directorio de trabajo");
		return -1;
	}

	/* descriptores standard deben ser cerrados (medida de seguridad) */

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	

	/* proceso daemon */
	/* bucle infinito del daemon */
	/* aqui debe escanear constantemente la PC para verificar si existen nuevos dispositivos USB 
	conectados y 
	 */

	while (1) {
		struct udev *udev;
		udev = udev_new();
		enumerar_disp_alm_masivo(udev);
      	sleep(5); 	
	}	


	/*prueba pequena */
	/*
	struct udev *udev;
	for(int i=0; i<=10; i++){
		udev = udev_new();
		enumerar_disp_alm_masivo(udev,logsdaemon);
		sleep(5); 
}
*/
	return -1; //borrar cuando creamos daemon
}
