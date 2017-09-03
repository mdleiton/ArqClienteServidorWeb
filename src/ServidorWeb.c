#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#define PORT 8888

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
struct Nodo{
  char* nombre;
  char* direccion_fisica;
  char* direccion_logica;
  struct Nodo *sgte;
};
struct USBnombrado{
  char* nombre;
  char* direccion_fisica;
  char* direccion_logica;
};
struct Nodo *primer= NULL;
struct Nodo *ultimo= NULL;
char* lista[5];
struct USBnombrados* nombrados[5];
int elementos=0;
void agregar(struct Nodo *nodo){
    nodo -> sgte = NULL;
    if (primer==NULL){
        primer=nodo;
    } else{
        ultimo -> sgte = nodo;
        ultimo = nodo;
    }
}

struct Nodo* agregarLista(char* nombre_usb, char* direcion_fisica_usb){
    struct Nodo * lista= malloc(sizeof(struct Nodo));
    lista -> nombre= nombre_usb;
    lista -> direccion_fisica= direcion_fisica_usb;
    //lista -> direccion_logica= direcion_logica_usb;
     agregar(lista);
     return lista;
   // printf("%s%s%s\n", lista->nombre,->direccion_fisica, lista->direccion_logica);
}

void recorrer(){
    struct Nodo *i = primer;
    while(i != NULL){
    //printf("%s%s%s\n", i->nombre,i->direccion_fisica, i->direccion_logica);
    i ->sgte;
    }

}

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
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)));
  strncpy(upload_datas, upload_data+1, strlen( upload_data)-2);
  r = jsmn_parse(&p,  upload_datas, strlen(upload_datas), t, 128);
  printf ("\nprocesando contenido del json.....\n");
  printf("%d,%s",r,upload_datas);
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
        printf("\n- %s: %.*s\n", "nodo",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }
      if (jsoneq( upload_datas, &t[i], "nombre") == 0) {
        sprintf(nombre,"%.*s",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);      
        printf("- %s: %.*s\n", "nombre",t[i+1].end-t[i+1].start-4, upload_datas + t[i+1].start+2);
        i++;
      }      
    }
  sprintf(elementolista,"%s-%s",nombre,nodo);      
  return elementolista;
}
int prcesandojson(const char *upload_data, int cantparametros, const char *s[]) {
  int r,i;
  jsmn_parser p;
  jsmntok_t t[128]; 
  jsmn_init(&p);
  char* upload_datas=malloc(sizeof(char)*(strlen( upload_data)));
  strncpy(upload_datas, upload_data+1, strlen( upload_data)-2);
  r = jsmn_parse(&p,  upload_datas, strlen(upload_datas), t, 128);
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
void iterarElemento(char *lista[]){
  printf("\n LISTA VIRTUAL SERVIDOR . \n");
  if(elementos==0) {
    printf("Lista vacia.No hay dispositivo USB nombrados. \n");
    return;
  }
  for (int i = 0; i < elementos; i++) {     
      printf("dispositivo : %s \n", lista[i]);
      }

}



/*MHD_Connection *connection es dada por libmicrohttpd daemon para mantener requerida inf relacionada a la conexion*/

int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
                          const char *upload_data, size_t *upload_data_size, void **con_cls){

	const char *page  = "<html><body>Servidor web Programacion de sistemas</body></html>";
	struct MHD_Response *response;
	int ret;
	//procesando solicitud tipo get
	if (0 == strcmp (method, "GET")){
	    printf("PROCESANDO SOLICUTUD GET \n");
	    MHD_get_connection_values (connection, MHD_HEADER_KIND, print_out_key,NULL);
	    const char* value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "data");
	    printf("PROCESANDO value: %s\n",value);
	 }
	//procesando solicitud tipo post
	if (0 == strcmp (method, "POST")){
   		printf ("PROCESANDO SOLICUTUD POST \n");
	    struct connection_info_struct *con_info = *con_cls;
	    if (*upload_data_size != 0){
		return MHD_YES;
	      }
	  }
	response = MHD_create_response_from_buffer (strlen (page),(void*) page, MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}

int main (){
  	struct MHD_Daemon *daemon;
	daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,&answer_to_connection,NULL,MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                             NULL, MHD_OPTION_END);
  	if (NULL == daemon) return 1;

	getchar ();

  	MHD_stop_daemon (daemon);
  	return 0;
}
