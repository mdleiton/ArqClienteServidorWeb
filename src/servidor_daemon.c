#include <sys/types.h>          /* some systems still require this */
#include <sys/stat.h>
#include <stdio.h>              /* for convenience */
#include <stdlib.h>             /* for convenience */
#include <stddef.h>             /* for offsetof */
#include <string.h>             /* for convenience */
#include <unistd.h>             /* for convenience */
#include <signal.h>             /* for SIG_ERR */ 
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
