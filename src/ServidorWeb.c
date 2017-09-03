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
