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
#define BUFFERING 100000

struct USBlista{
  char* nombre;
  char* nodo;
  char* montaje;
  char* sci;
  char* VendoridProduct;
};
struct USBnombrado{
  char* nombre;
  char* direccion_fisica;
  char* direccion_logica;
};

struct USBnombrado* nombrados[5];
int elementos=0;

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

char* init_cliente(char *solicitud){ 
  int sockfd,filefd; 
  int puerto =8888;
  struct sockaddr_in direccion_cliente;
  memset(&direccion_cliente, 0, sizeof(direccion_cliente)); 
  direccion_cliente.sin_family = AF_INET;   //IPv4
  direccion_cliente.sin_port = htons(puerto);   //Convertimos el numero de puerto al endianness de la red
  direccion_cliente.sin_addr.s_addr = inet_addr("127.0.0.1") ;  //Nos tratamos de conectar a esta direccion
  if (( sockfd = connect_retry( direccion_cliente.sin_family, SOCK_STREAM, 0, (struct sockaddr *)&direccion_cliente, sizeof(direccion_cliente))) < 0) { 
    printf("falló conexión con el proceso daemonUSB . \n Verifique que no existen ningun proceso daemonUSB ejecutando y vuelva a ejecutar el daemon\n"); 
    return "\"str_error\":\"ERROR:falló conexión con el proceso daemonUSB . Verifique que no existen ningun proceso daemonUSB ejecutando y vuelva a ejecutar el daemon\"\n";
  } 
  send(sockfd,solicitud,BUFLEN,0);
  printf("Solicitud enviada proceso daemon: %s \n",solicitud);
  int n=0;    
  char *file = malloc(BUFFERING*sizeof(char *));
  memset(file,0,BUFFERING);
  printf("procesando respuesta del daemon\n");
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
  if (n <= 0){
    printf("Error al transferir la informacion\n");
    close(sockfd);
    return "\"str_error\":\"ERROR:al transferir la informacion\"\n";
  }  
  memset(file,0,BUFFERING);
  free(file);
  close(sockfd);
  return "\"str_error\":\"ERROR:no se recibio respuesta del daemon\"\n";
}

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
// funciones para procesar json
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if ((int) strlen(s) == tok->end - tok->start-4 && strncmp(json + tok->start+2, s, tok->end - tok->start-4) == 0) {
    return 0;
  }
  return -1;
}

char* procesandojsonnombrar(const char *upload_data) {
  char *elementolista=malloc(sizeof(char)*(60));  
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* nodo=malloc(sizeof(char)*(10));
  char *nombre=malloc(sizeof(char)*(50));
   printf("\n aaaaaaaaa%s\n",upload_data);
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
  printf("%d,%s",r,re);
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
        printf("\n - %s: %.*s\n", "nodo",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }
      if (jsoneq( upload_datas, &t[i], "nombre") == 0) {
        sprintf(nombre,"%.*s",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);      
        printf("\n - %s: %.*s\n", "nombre",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }      
    }

  struct USBnombrado *usb=malloc(sizeof(struct USBnombrado));
  usb->nombre=nombre;
  usb->direccion_fisica=nodo;
  nombrados[elementos]=usb;
  elementos++;     
  sprintf(elementolista,"%s-%s",usb->direccion_fisica, usb->nombre); 
  return elementolista;
}
int prcesandojson(const char *upload_data, int cantparametros, const char *s[]) {
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)));
  char * respx=malloc(8*sizeof(char *));
      int j=0;
      for(int i=0;i<strlen(upload_data);i++){
        if(upload_data[i]=='{' && j==0){
          i++;
          while(upload_data[i]!='}'){
            upload_datas[j]=upload_data[i];
            i++;
            j++;
          }
        }
      }
      char * re=malloc((j-1)*sizeof(char *));
      memset(re,0,j-1);
      re=upload_datas;
  
  r = jsmn_parse(&p,  re, strlen(re), t, 128);
  printf ("\nprocesando contenido del json.....\n");
  /* Assume the top-level element is an object */
  if (r < 0) {
    printf("Failed to parse JSON: %d\n", r);
    return 0;
  }
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object no expected\n");
    return 0;
  }
  for(int token=0;token < cantparametros; token++){
    for (i = 1; i < r; i++) {
      if (jsoneq( upload_datas, &t[i], s[token]) == 0) {
        printf("- %s: %.*s\n", s[token],t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }
    }
}
  return 1;
}

static int iterar_encabezado (void *cls, enum MHD_ValueKind kind, const char *key, const char *value){
  printf ("Encabezado %s: %s\n", key, value);
  return MHD_YES;
}

void iterar(struct USBnombrado *lista[]){
  printf("\n LISTA VIRTUAL SERVIDOR . \n");
  if(elementos==0) {
    printf("Lista vacia.No hay dispositivo USB nombrados. \n");
    return;
  }
  for (int i = 0; i < elementos; i++) {     
      printf("dispositivo N%d| nombre:%s,direccion fisica:%s,direccion logica:%s \n", i+1,lista[i]->nombre,lista[i]->direccion_fisica,lista[i]->direccion_logica);
      }
}



/*MHD_Connection *connection es dada por libmicrohttpd daemon para mantener requerida inf relacionada a la conexion*/
int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
                          const char *upload_data, size_t *upload_data_size, void **con_cls){
  //procesando los tipos de solicitudes
  char* jsonresp=malloc(BUFFERING*sizeof(char *));
  memset(jsonresp,0,BUFFERING);
  char* solicitud=malloc(BUFFERING*sizeof(char *));
  memset(solicitud,0,BUFFERING);
  if (0 == strcmp (method,MHD_HTTP_METHOD_GET)  && !strncasecmp(url, "/listar_dispositivos", 19)){
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
       return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
     }
    }
     //solicitud nombrar_dispositivo
  }else if (0 == strcmp (method, "POST") && !strncasecmp(url, "/nombrar_dispositivo", 17)){
    int i,r;
    if(upload_data==NULL){
      return MHD_YES;
    }
    if(upload_data!=NULL){
      iterar(nombrados);
      printf ("\nNueva  %s solicitud en  %s con  version %s \n", method, url, version);
      MHD_get_connection_values (connection, MHD_HEADER_KIND, iterar_encabezado,NULL);
      printf ("obteniendo json con información............\n");
      char * resp=procesandojsonnombrar(upload_data);
      sprintf(solicitud, "%s-%s","obtenerdireccion",resp);
      char *respuesta=init_cliente(solicitud);
      if(strstr(respuesta, "no se encuentra")!=NULL){
        sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \n"
                    "\"status\": \"-1 \", %s}",respuesta);
        return enviar_respuesta (connection, jsonresp,400); 
      }else{
        nombrados[elementos-1]->direccion_logica=respuesta;
        iterar(nombrados);
        sprintf(jsonresp,"{\"solicitud\": \"nombrar_dispositivo\", \"nombre\":\"%s\" , \n"
                      "\"nodo\":\"%s\" ,\"status\": \"0\", \"str_error\" : 0}",respuesta,respuesta);
        return enviar_respuesta (connection, jsonresp, MHD_HTTP_OK);
      }
    }
    }else{
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



