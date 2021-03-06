#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jsmn.h"
#include <sys/stat.h>
#include <stddef.h>             /* for offsetof */
#include <unistd.h>             /* for convenience */
#include <netdb.h> 
#include <errno.h> 
#include <syslog.h> 
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#define BUFLEN 1024
#define MAXSLEEP 24
#define BUFFERING 1000000

struct USBlista{
  char* nombre;
  char* nodo;
  char* montaje;
  char* sci;
  char* VendoridProduct;
};
struct manejoColaJson{
  char* info;
  char* stringjson;
};

struct USBnombrado{
  char* nombre;
  char* direccion_fisica;
  char* direccion_logica;
};

struct USBnombrado* nombrados[5];  //donde se almacena la lista de dispositivos con nombre definido por el usuario.
int elementos=0;
struct USBlista* usblista[5];   //donde se almacena un historial de todos los dispositivos conectados.
int usbelementos=0;

//establece conexion con el proceso daemon
int connect_retry( int domain, int type, int protocol,  const struct sockaddr *addr, socklen_t alen){
  int numsec, fd; /* * Try to connect with exponential backoff. */ 
  for (numsec = 1; numsec <= MAXSLEEP; numsec++) { 
    if (( fd = socket( domain, type, protocol)) < 0) 
      return(-1); 
    if (connect( fd, addr, alen) == 0) { /* * Conexión aceptada. */ 
      return(fd); 
    } 
    close(fd);  
    sleep(1);
  } 
  return(-1); 
}

//envia solicitudes al daemon y recibe las respectivas respuestas
char* init_cliente(char *solicitud){ 
  int sockfd,filefd; 
  int puerto =8888;
  int n=0; 
  struct sockaddr_in direccion_cliente;
  memset(&direccion_cliente, 0, sizeof(direccion_cliente)); 
  direccion_cliente.sin_family = AF_INET;   //IPv4
  direccion_cliente.sin_port = htons(puerto);   //Convertimos el numero de puerto al endianness de la red
  direccion_cliente.sin_addr.s_addr = inet_addr("127.0.0.1") ;  //Nos tratamos de conectar a esta direccion
  if (( sockfd = connect_retry( direccion_cliente.sin_family, SOCK_STREAM, 0, (struct sockaddr *)&direccion_cliente, sizeof(direccion_cliente))) < 0) { 
    printf("falló conexión con el proceso daemonUSB . \n Verifique que no existen ningun proceso daemonUSB ejecutando y vuelva a ejecutar el daemon\n"); 
    return "\"str_error\":\"ERROR:falló conexión con el proceso daemonUSB . Verifique que no existen ningun proceso daemonUSB ejecutando y vuelva a ejecutar el daemon\"\n";
  } 

  if(strstr(solicitud, "escribir_archivo")!=NULL){
    printf("Solicitud enviada proceso daemon:\n %s \n",solicitud);
    send(sockfd,solicitud,strlen(solicitud),0);
    printf("procesando respuesta del daemon\n");
    char *num = (char*)malloc(6*sizeof(char *));
    memset(num,0,6);
    recv(sockfd, num, 6,0);
    printf("\n%d\n", atoi(num));
    if(atoi(num)==0){
        return "\"str_error\":\"ERROR:al recibir respuesta del daemon\"\n";
    }
    char *file = (char*)malloc(atoi(num)+2*sizeof(char *));
    memset(file,0,atoi(num)+2);
    if((n=recv(sockfd, file, atoi(num)+2,0))>0){
      if (strstr(file, "ERROR") != NULL) {
        printf("ERROR: al recibir respuesta del daemon\n");
        close(sockfd);
        return "\"str_error\":\"ERROR:al recibir respuesta del daemon\"\n";
      }
      printf("respuesta del daemonUSB:\n %s\n",file);
      close(sockfd);
      return file;
    }  
    return "\"str_error\":\"ERROR:no se recibio respuesta del daemon\"\n";
  }
  send(sockfd,solicitud,strlen(solicitud),0);
  printf("Solicitud enviada proceso daemon:\n %s \n",solicitud);
  printf("procesando respuesta del daemon\n");
  char *file = malloc(BUFFERING*sizeof(char *));
  memset(file,0,BUFFERING);
  for(int i=0;i<100;i++){  
    if((n=recv(sockfd, file, BUFFERING,0))>0){
      if (strstr(file, "ERROR") != NULL) {
        printf("ERROR: al recibir respuesta del daemon\n");
        close(sockfd);
        return "\"str_error\":\"ERROR:al recibir respuesta del daemon\"\n";
      }
      printf("respuesta del daemonUSB:\n %s\n",file);
      close(sockfd);
      return file;
    }  
    usleep(1000);
  }
  return "\"str_error\":\"ERROR:no se recibio respuesta del daemon\"\n";
}

//se encarga de enviar el json de respuesta a las solicitudes del cliente.
static int enviar_respuesta(struct MHD_Connection *connection, const char *page, int statuscodigo){
  int ret;
  struct MHD_Response *response;
  response =MHD_create_response_from_buffer (strlen (page), (void *) page,MHD_RESPMEM_PERSISTENT);
  if (!response) return MHD_NO;
  MHD_add_response_header(response,"Content-Type","application/json");
  ret = MHD_queue_response (connection, statuscodigo, response);
  MHD_destroy_response (response);
  return ret;
}

//esta funcion permite obtener la direccion de los dispositivos a partir del nombre
char* obtenerDireccion(char* nombre){
  if(elementos==0) {
    printf("No existe algun dispositivo nombrado . \n");
    return"\"str_error\":\"ERROR:No existe algun dispositivo nombrado . \"";
  }
  for (int i = 0; i < elementos; i++) {      
    if(strstr(nombrados[i]->nombre,nombre )!=NULL){
      return  nombrados[i]->direccion_logica;
      } 
  } 
  printf("No existe algun dispositivo nombrado con ese nombre. \n");
  return"\"str_error\":\"ERROR:No existe algun dispositivo nombrado con ese nombre. \""; 
}
/*permite obtener la direccion a partir de la direccion nodo*/
char* obtenernombre(char* direccion){
  if(elementos== 0){
    return NULL;
  }
  for (int i = 0; i < elementos; i++) {      
    if(strstr(nombrados[i]->direccion_fisica, direccion)!=NULL){
      return  nombrados[i]->nombre;
      } 
  } 
  return NULL;
}

/* funciones para procesar json. este es cuando el json viene con cosas adicionales del cliente o del daemon pequeno bug
se hace uso de la libreria. jsmn
*/
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if ((int) strlen(s) == tok->end - tok->start-4 && strncmp(json + tok->start+2, s, tok->end - tok->start-4) == 0) {
    return 0;
  }
  return -1;
}

//cuando el json "limpio" esta correctamente hecho. se hace uso de la libreria. jsmn
static int jsonlimpio(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

//procesa json de la solicitud nombrar dispositivo
char* procesandojsonnombrar(const char *upload_data) {
  char *elementolista=malloc(sizeof(char)*(60));  
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* nodo=malloc(sizeof(char)*(10));
  char *nombre=malloc(sizeof(char)*(50));
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)-2));
  memset(upload_datas,0,strlen( upload_data)-2);
  int j=0;
  for(int i=0;i<strlen(upload_data);i++){
    if(upload_data[i]=='{' && j==0){
      while(upload_data[i]!='}'){
        upload_datas[j]=upload_data[i];
        i++;
        j++;
      }
      upload_datas[j]=upload_data[i];
    }
  }
  char * re=malloc((j-1)*sizeof(char *));
  memset(re,0,j-1);
  re=upload_datas;
  r = jsmn_parse(&p,  re, strlen(re), t, 128);
  printf ("\nprocesando contenido del json.....\n");
  printf("json recibido %d:\n %s \n",r,re);
  /* Assume the top-level element is an object */
  if (r < 0) {
    return NULL;
  }
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return NULL;
  }
    for (i = 1; i < r; i++) {
      if (jsoneq( upload_datas, &t[i], "nodo") == 0) {
        sprintf(nodo,"%.*s",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);      
        printf("\n - %s: %.*s", "nodo",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }
      if (jsoneq( upload_datas, &t[i], "nombre") == 0) {
        sprintf(nombre,"%.*s",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);      
        printf("\n - %s: %.*s", "nombre",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }      
    }
    if(obtenernombre(nodo)!=NULL) return NULL;
  struct USBnombrado *usb=malloc(sizeof(struct USBnombrado));
  usb->nombre=nombre;
  usb->direccion_fisica=nodo;
  nombrados[elementos]=usb;
  elementos++;     
  sprintf(elementolista,"%s-%s",usb->direccion_fisica, usb->nombre); 
  return elementolista;
}

//procesa json de la solicitud escribir archivo
char* procesandojsonEscribir(const char *upload_data) {
  char *elementolista=malloc(sizeof(char)*(BUFFERING)); 
  memset(elementolista,0,BUFFERING);
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* nombre_archivo=malloc(sizeof(char)*(15));
  char *nombre=malloc(sizeof(char)*(50));
  char *solicitud=malloc(sizeof(char)*(50));
  char *tamano_contenido=malloc(sizeof(char)*(14));
  int tamano;
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)-2));
  memset(upload_datas,0,strlen( upload_data)-2);
  int j=0;
  for(int i=0;i<strlen(upload_data);i++){
    if(upload_data[i]=='{' && j==0){
      while(upload_data[i]!='}'){
        upload_datas[j]=upload_data[i];
        i++;
        j++;
      }
      upload_datas[j]=upload_data[i];
    }
  }
  char * re=malloc((j-1)*sizeof(char *));
  memset(re,0,j-1);
  re=upload_datas;
  r = jsmn_parse(&p,  re, strlen(re), t, 128);
  printf ("\nprocesando contenido del json.....\n");
  printf("json recibido %d:\n %s \n",r,re);
  /* Assume the top-level element is an object */
  if (r < 0) {
    return NULL;
  }
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return NULL;
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_datas, &t[i], "nombre_archivo") == 0) {
      sprintf(nombre_archivo,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      printf("\n - %s: %.*s", "nombre_archivo",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);
      i++;
      sprintf(elementolista,"%s",nombre_archivo); 
    }
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_datas, &t[i], "nombre") == 0) {
      sprintf(nombre,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      printf("\n - %s: %.*s", "nombre",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);
      i++;
    }
    if (jsonlimpio( upload_datas, &t[i], "tamano_contenido") == 0) {
      sprintf(tamano_contenido,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      printf("\n - %s: %.*s", "tamano_contenido",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);
      i++;
      sprintf(elementolista,"%s|%s",elementolista,tamano_contenido); 
    }
    if (jsonlimpio( upload_datas, &t[i], "solicitud") == 0) {
      sprintf(solicitud,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      printf("\n - %s: %.*s", "solicitud",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);
      i++;
      sprintf(elementolista,"%s|%s",elementolista,solicitud); 
    }
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_datas, &t[i], "contenido") == 0) {
      tamano=atoi(tamano_contenido);
      char *contenido=malloc(sizeof(char)*(tamano));
      sprintf(contenido,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
       i++;
      sprintf(elementolista,"%s|%s",elementolista,contenido); 
    }
  }
  char* direccion=obtenerDireccion(nombre);
  if(strstr(direccion, "str_error")!=NULL) return direccion;
  sprintf(elementolista,"%s|%s|%s|",elementolista,nombre,direccion); 
  return elementolista;
 
}

//procesa json de la solicitud escribir archivo
char* procesandojsonLeer(const char *upload_data) {
  printf("%s\n",upload_data );
  char *elementolista=malloc(sizeof(char)*(BUFLEN)); 
  memset(elementolista,0,BUFLEN);
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* nombre_archivo=malloc(sizeof(char)*(15));
  char *nombre=malloc(sizeof(char)*(50));
  char *solicitud=malloc(sizeof(char)*(50));
  int j=0;
  r = jsmn_parse(&p, upload_data, strlen(upload_data), t, 128);
  printf ("\nprocesando contenido del json.....\n");
  printf("json recibido %d:\n %s \n",r,upload_data);
  /* Assume the top-level element is an object */
  if (r < 0) {
    return NULL;
  }
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return NULL;
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_data, &t[i], "nombre_archivo") == 0) {
      sprintf(nombre_archivo,"%.*s",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);      
      printf("\n - %s: %.*s", "nombre_archivo",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);
      i++;
      sprintf(elementolista,"%s",nombre_archivo); 
    }
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_data, &t[i], "nombre") == 0) {
      sprintf(nombre,"%.*s",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);      
      printf("\n - %s: %.*s", "nombre",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);
      i++;
    }
    if (jsonlimpio( upload_data, &t[i], "solicitud") == 0) {
      sprintf(solicitud,"%.*s",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);      
      printf("\n - %s: %.*s", "solicitud",t[i+1].end-t[i+1].start, upload_data + t[i+1].start);
      i++;
      sprintf(elementolista,"%s|%s",elementolista,solicitud); 
    }
  }
  char* direccion=obtenerDireccion(nombre);
  if(strstr(direccion, "str_error")!=NULL) return direccion; //lo que devuelve es el mjs de error
  sprintf(elementolista,"%s|%s|%s|",elementolista,nombre,direccion);
  printf("%s\n",elementolista ); 
  return elementolista;
 
}

//permite ver el header de la solicitud en el servidor
static int iterar_encabezado (void *cls, enum MHD_ValueKind kind, const char *key, const char *value){
  printf ("Encabezado %s: %s\n", key, value);
  return MHD_YES;
}

//permite visualizar un historial de los dispositivos conectados cuando se hizo alguna solicitud 
void iterar(struct USBnombrado *lista[]){
  printf("\n  VIRTUAL SERVIDOR DE DISPOSITIVO NOMBRADOS. \n");
  if(elementos==0) {
    printf("Lista vacia.No hay dispositivo USB nombrados. \n");
    return;
  }
  for (int i = 0; i < elementos; i++) {     
      printf("dispositivo N %d | nombre:%s,direccion fisica:%s,direccion logica:%s \n", i+1,lista[i]->nombre,lista[i]->direccion_fisica,lista[i]->direccion_logica);
      }
}

//permite visualizar una lista de los dispositivos nombrados
void iterarlistado(struct USBlista *lista[]){
  printf("\n HISTORIAL VIRTUAL SERVIDOR DE DISPOSITIVO . \n");
  if(usbelementos==0) {
    printf("Lista vacia.No hay dispositivo USB . \n");
    return;
  }
  for (int i = 0; i < usbelementos; i++) {     
    printf("USB N %d | nombre:%s,nodo:%s,montaje:%s,sci:%s,VendoridProduct :%s \n", i+1,lista[i]->nombre,lista[i]->nodo,lista[i]->montaje,lista[i]->sci,lista[i]->VendoridProduct);
  }
}

//recibe la respuesta del daemon sobre lista de dispositivos y los agrega a una lista como historial 
void procesandolistaUSB(const char *upload_data) {
  char *elementolista=malloc(sizeof(char)*(60));  
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* nodo=malloc(sizeof(char)*(10));
  char *montaje=malloc(sizeof(char)*(50));
  char *sci=malloc(sizeof(char)*(50));
  char *VendoridProduct=malloc(sizeof(char)*(50));
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)-2));
  memset(upload_datas,0,strlen( upload_data)-2);
  int j=0;
  for(int i=0;i<strlen(upload_data);i++){
    if(upload_data[i]=='{' && j==0){
      while(upload_data[i]!='}'){
        upload_datas[j]=upload_data[i];
        i++;
        j++;
      }
      upload_datas[j]=upload_data[i];
    }
  }
  char * re=malloc((j-1)*sizeof(char *));
  memset(re,0,j-1);
  re=upload_datas;
  r = jsmn_parse(&p,  upload_data, strlen(upload_data), t, 128);
  if (r < 0) {
    return ;
  }
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return ;
  }
  for (i = 1; i < r; i++) {
    if (jsonlimpio( upload_data, &t[i], "nodo") == 0) {
      sprintf(nodo,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      i++;
    }
    if (jsonlimpio( upload_data, &t[i], "montaje") == 0) {
      sprintf(montaje,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      i++;
    }  
    if (jsonlimpio( upload_data, &t[i], "scsi") == 0) {
      sprintf(sci,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      i++;
    }  
    if (jsonlimpio( upload_data, &t[i], "Vendor:idProduct") == 0) {
      sprintf(VendoridProduct,"%.*s",t[i+1].end-t[i+1].start, upload_datas + t[i+1].start);      
      i++;
    }         
  }
  char * nombre=obtenernombre(nodo);
   struct USBlista *usb2=malloc(sizeof(struct USBlista));
  usb2->nombre=nombre;
  usb2->nodo=nodo;
  usb2->montaje=montaje;
  usb2->sci=sci;
  usb2->VendoridProduct=VendoridProduct;
  usblista[usbelementos]=usb2;
  usbelementos++;     
  return;
}

//cuando existen dos o mas dispositivos conectados permite dividir el json.
void tokenizarsolicitud(char* listausb){
  char* usb1=malloc(BUFLEN*sizeof(char*));
  if(strstr(listausb,"},")>0){
    const char delimitadores[2] = "}";
    char *token;
    token = strtok(listausb, delimitadores);
    int i=0;
    while( token != NULL ) {
      sprintf(usb1,"%s}",token);
      if(i>0){
      procesandolistaUSB(usb1+1);
      printf("%s\n", usb1+1);
      }else{
      procesandolistaUSB(usb1);
      printf("%s\n", usb1);
      i++;
    }
      token = strtok(NULL, delimitadores);
      
   }
  }else{
    procesandolistaUSB(listausb);
  }
}

//permite asignar un nombre tanto en la lista de historial como en la lista de dispositivos nombrados.
struct USBlista* asignarnombre(char * solicitud){
  char * respx=malloc(10*sizeof(char *));
  char * resp2=malloc(20*sizeof(char *));
    int j=0,z=0,w=0;
    for(int i=0;i<strlen(solicitud);i++){
      if(solicitud[i]=='-' && j==0){
        i++;
        while(solicitud[i]!='-'){
          respx[j]=solicitud[i];
          i++;
          j++;
        }
        z++;
        i++;
      }
      if(z==1){
        resp2[w]=solicitud[i];
        w++;
      }
    }
  char * r=malloc((j-2)*sizeof(char *));
  memset(r,0,j-2);
  char * nombre=malloc((w)*sizeof(char *));
  memset(nombre,0,w-1);
  r=respx;
  nombre=resp2;
  for (int i = 0; i < usbelementos; i++) {  
    if(strstr(r,usblista[i]->nodo )!=NULL && usblista[i]->nombre==NULL){
      usblista[i]->nombre=nombre;
      return  usblista[i];
    } 
  }
  return NULL;
}

/*
megafuncion que se encarga de procesar cada de una de las solicitudes que reciba el servidorweb.
a su vez se encarga de procesar y redireccionar la solicitud al daemon y luego recibir la respuesta ,
 esta respuesta procesarla y enviar al cliente.
*/
int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
                          const char *upload_data, size_t *upload_data_size, void **con_cls){
  
  //variables usadas para todo tipo de solicitud
  char* jsonresp=malloc(BUFFERING*sizeof(char *));
  memset(jsonresp,0,BUFFERING);
  char* solicitud=malloc(BUFFERING*sizeof(char *));
  memset(solicitud,0,BUFFERING);

  //procesando los tipos de solicitudes

  //solicitudes por metodo o versiones invalidas

  //filtrando solo solicitudes validas
  if (0 == strcmp (method,"PUT")  || 0 == strcmp (method,"DELETE") || 0 == strcmp (method,"HEAD") || 0 == strcmp (method,"OPTIONS")){
    //501 Not Implemented
    printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
    MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
    char * resp="\"str_error\":\"ERROR: 501 Not Implemented\"";
    sprintf(jsonresp,"{\"solicitud\": \"%s\", \n"
             "\"status\": \"-1\", %s }",url,resp);
    return enviar_respuesta (connection, jsonresp,501);
  }
  if (0 != strcmp (version,"HTTP/1.1")){
    //505 HTTP Version Not Supported
    printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
    MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
    char * resp="\"str_error\":\"ERROR:505 HTTP Version Not Supported\"";
    sprintf(jsonresp,"{\"solicitud\": \"%s\", \n"
             "\"status\": \"-1 \", %s }",url,resp);
    return enviar_respuesta (connection, jsonresp,505);
  }

  //procesando solicitudes validas 

  //solicitud listar dispositivos
  if (0 == strcmp (method,MHD_HTTP_METHOD_GET)  && !strncasecmp(url, "/listar_dispositivos", 19)){
    memset(usblista,0,5);
    usbelementos=0;
    sprintf(solicitud, "%s-%s",method,"listar_dispositivos");
    printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
    MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
    char *resp=init_cliente(solicitud);
    if(strstr(resp, "str_error")!=NULL ){
       sprintf(jsonresp,"{\"solicitud\": \"listar_dispositivos\", \n"
                      "\"status\": \"-1 \", %s}",resp);
       return enviar_respuesta (connection, jsonresp,400); 
    }else{
      if(strlen(resp)<=1){
        sprintf(jsonresp,"{\"solicitud\": \"listar_dispositivos\", \"dispositivos\": [%s ], \n"
                      "\"status\": \"0\", \"str_error\" :\"no existen dispositivos actualmente conectados\" }",resp);
       return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK); 
      }else{
       sprintf(jsonresp,"{\"solicitud\": \"listar_dispositivos\", \"dispositivos\": [%s ], \n"
                      "\"status\": \"0\", \"str_error\" : 0}",resp);
       tokenizarsolicitud(resp);
       iterarlistado(usblista);
      return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
      }
    }
  }//solicitud nombrar_dispositivo
  else if (0 == strcmp (method, "POST") && !strncasecmp(url, "/nombrar_dispositivo", 17)){
    int i,r;
    struct manejoColaJson *cola = NULL;
    cola = (struct manejoColaJson*)*con_cls;
    if(cola == NULL) {
      cola = malloc(sizeof(struct manejoColaJson));
      cola->info = 0;
      *con_cls = cola;
      return MHD_YES;
    }
    if(*upload_data_size != 0) {
      cola->stringjson= malloc(*upload_data_size + 1);
      strncpy(cola->stringjson, upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    }else {
      printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
      iterar(nombrados);
      MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
      printf ("obteniendo json con información............\n");
      sprintf(solicitud, "%s-%s","GET","listar_dispositivos");
      char *respuestadaemon=init_cliente(solicitud);
      if(strstr(respuestadaemon, "str_error")==NULL && strlen(respuestadaemon)>=2){
        tokenizarsolicitud(respuestadaemon);
        char * resp=procesandojsonnombrar(cola->stringjson);
        if(resp!=NULL){
          char *ss=malloc(BUFFERING*sizeof(char *));
          sprintf(ss, "%s-%s","obtenerdireccion",resp);
          struct USBlista *usb2=malloc(sizeof(struct USBlista));
          usb2=asignarnombre(ss);
          if(usb2!=NULL){
          nombrados[elementos-1]->direccion_logica=usb2->montaje;
          iterar(nombrados);
          sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \"nombre\":\"%s\" , \n"
                        "\"nodo\":\"%s\" ,\"status\": \"0\", \"str_error\" : 0}",nombrados[elementos-1]->nombre,nombrados[elementos-1]->direccion_fisica);
          return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
          }else{
            printf("No existe un dispositivo conectado con ese nodo");
             sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \"nombre\":\"%s\" , \n"
                        "\"nodo\":\"%s\" ,\"status\": \"-1\", \"str_error\" :\"No existe un dispositivo conectado con ese nodo  o ya tiene nombre asignado\"}",nombrados[elementos-1]->nombre,nombrados[elementos-1]->direccion_fisica);
          return enviar_respuesta (connection, jsonresp, 404);
          }
        }else{
          sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \n"
                      "\"status\": \"-1 \",\"str_error\" :\"error : formato json erroneo o dispositivo ya tiene nombre asignado\" }");
          return enviar_respuesta (connection, jsonresp,400); 
        }
      }else{
        if(strlen(respuestadaemon)>=2){
          sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \n"
                      "\"status\": \"-1 \", %s}",respuestadaemon);
          return enviar_respuesta (connection, jsonresp,400); 
        }else{
          sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \n"
                      "\"status\": \"-1 \",\"str_error\" :\"no existen dispositivos actualmente conectados\" }");
          return enviar_respuesta (connection, jsonresp,400); 
        }
      }   
    }
  }//solicitud escribir_archivo
  else if (0 == strcmp (method, "POST") && !strncasecmp(url, "/escribir_archivo", 17)){
    int i,r;
    struct manejoColaJson *cola = NULL;
    cola = (struct manejoColaJson*)*con_cls;
    if(cola == NULL) {
      cola = malloc(sizeof(struct manejoColaJson));
      cola->info = 0;
      *con_cls = cola;
      return MHD_YES;
    }
    if(*upload_data_size != 0) {
      cola->stringjson= malloc(*upload_data_size + 1);
      strncpy(cola->stringjson, upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    }else {
      printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
      MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
      printf ("obteniendo json con información........\n");
      char * solicitud=procesandojsonEscribir(cola->stringjson);
      if(strstr(solicitud, "str_error")!=NULL){
        sprintf(jsonresp,"{\"solicitud\": \"escribir_archivo\", \n"
                      "\"status\": \"-1 \", %s }",solicitud);
          return enviar_respuesta (connection, jsonresp,404); 
      }
      printf ("\n\ncompleto procesamiento json.... enviando al daemon........\n");
      char *respuestadaemon=init_cliente(solicitud);
      if(strstr(respuestadaemon, "str_error")==NULL){
        //no hay errores desde el daemon el daemon me devuelve el json hasta sin los campos status , ni str_error,sin coma
        sprintf(jsonresp,"%s\"status\": \"0\", \"str_error\" : 0}",respuestadaemon);
        return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
        }else{
          sprintf(jsonresp,"{\"solicitud\": \"escribir_archivo\", \n"
                      "\"status\": \"-1 \",%s }",respuestadaemon);
          return enviar_respuesta (connection, jsonresp,400); 
        }
    }
  }//sino ninguna de las solicitudes anteriores debe retorna mensaje que no existe 
  else if (0 == strcmp (method, "GET") && !strncasecmp(url, "/leer_archivo", 13)){
    int i,r;
    struct manejoColaJson *cola = NULL;
    cola = (struct manejoColaJson*)*con_cls;
    if(cola == NULL) {
      cola = malloc(sizeof(struct manejoColaJson));
      cola->info = 0;
      *con_cls = cola;
      return MHD_YES;
    }
    if(*upload_data_size != 0) {
      cola->stringjson= malloc(*upload_data_size + 1);
      strncpy(cola->stringjson, upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    }else {
      printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
      MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
      printf ("obteniendo json con información........\n");
      char * solicitud=procesandojsonLeer(cola->stringjson);
      printf("sss%s\n", cola->stringjson);
      if(solicitud==NULL){
        sprintf(jsonresp,"{\"solicitud\": \"leer_archivo\", \n"
                      "\"status\": \"-1 \", \"status\": \"ERROR: formato json incorrecto \"}");
          return enviar_respuesta (connection, jsonresp,404); 
      }
      if(strstr(solicitud, "str_error")!=NULL){
        sprintf(jsonresp,"{\"solicitud\": \"leer_archivo\", \n"
                      "\"status\": \"-1 \", %s }",solicitud);
          return enviar_respuesta (connection, jsonresp,404); 
      }
      printf ("\n\ncompleto procesamiento json.... enviando al daemon........\n");
      char *respuestadaemon=init_cliente(solicitud);
      if(strstr(respuestadaemon, "str_error")==NULL){
        //no hay errores desde el daemon el daemon me devuelve el json hasta sin los campos status , ni str_error,sin coma
        sprintf(jsonresp,"%s,\"status\": \"0\", \"str_error\" : 0}",respuestadaemon);
        return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
        }else{
          sprintf(jsonresp,"{\"solicitud\": \"leer_archivo\", \n"
                      "%s }",respuestadaemon);
          return enviar_respuesta (connection, jsonresp,400); 
        }
    }
  }//sino ninguna de las solicitudes anteriores debe retorna mensaje que no existe 
  else{
      //404 Not Found
      printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
      MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
      char * resp="\"str_error\":\"ERROR 404 Not Found\"";
      sprintf(jsonresp,"{\"solicitud\": \"%s\", \n"
             "\"status\": \" -1\", %s }",url,resp);
      int r=enviar_respuesta (connection, jsonresp,404); 
      printf("%d \n",r) ;
      return r;
    }
  return MHD_NO;
}

int main (int argc, char *argv[]){
  if(argc != 2){
    printf("Uso: ./bin/ServidorWeb <puerto>\n");
    exit(-1);
  }
  int puerto = atoi(argv[1]);
  struct MHD_Daemon *daemon;
   daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, puerto, NULL, NULL, &answer_to_connection, NULL,MHD_OPTION_END);
  if (NULL == daemon) return 1;

  (void) getchar ();

  MHD_stop_daemon (daemon);
  return 0;
}



