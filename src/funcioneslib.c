#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <mntent.h>
#include <errno.h>
#include <string.h>
typedef struct {
 char* nombre;
 char* direccion_fisica;
 char* direccion_logica;
 struct Nodo *sgte;
} Nodo;
 
Nodo *primer= NULL;
Nodo *ultimo= NULL;

void agregar(Nodo *nodo){
    nodo -> sgte = NULL;
    if (primer==NULL){
        primer=nodo;
    } else{
       
        ultimo -> sgte = nodo;
        ultimo = nodo;
    }
}
void agregarLista(char* nombre_usb, char* direcion_fisica_usb, char* direcion_logica_usb){
    Nodo * lista= malloc(sizeof(Nodo));
    lista -> nombre= nombre_usb;
    lista -> direccion_fisica= direcion_fisica_usb;
    lista -> direccion_logica= direcion_logica_usb;
     agregar(lista);
   // printf("%s%s%s\n", lista->nombre,->direccion_fisica, lista->direccion_logica);
}

void recorrer(){
    Nodo *i = primer;
    while(i != NULL){
    //printf("%s%s%s\n", i->nombre,i->direccion_fisica, i->direccion_logica);
    i ->sgte;
    }

}

/* lee el archivo del pendrive */
char* leer_archivo(char* direccion, char* nombre_archivo){
	FILE *archivo;
	int caracter;
	char resultado[1000];
	char* texto_final;
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


/*escribir archivo en el pendrive*/
void escribir(char* direccion, char* nombre_archivo, int tamano, char* contenido){
	int MAX=tamano;	
	char resultado[1000];
	char* texto_final;
	sprintf(resultado,"%s/%s", direccion,nombre_archivo);

    char cadena[MAX];
    sprintf(cadena,"%s%s", cadena,contenido);
    FILE* fichero;
    fichero = fopen(resultado, "wt");
    fputs(cadena, fichero);
    fclose(fichero);
    printf("Proceso completado");
}


/* permite presentar toda la estructura de una dispositivo usb conectado a la pc*/

void presentar_estructuraMNTENT(const struct mntent *fs){
	printf("nodo :%s \n direccion logica :%s \n %s \n %s \n %d \n %d\n",
		fs->mnt_fsname,  /* name of mounted filesystem(es el nodo del dispositivo) */
		fs->mnt_dir,    /* filesystem path prefix (el directorio donde está montado.)*/
		fs->mnt_type,	/* mount type  */
		fs->mnt_opts,	/* mount options  */
		fs->mnt_freq,	/* dump frequency in days */
		fs->mnt_passno);	/* pass number on parallel fsck */
}

struct udev_device* obtener_hijo(struct udev* udev, struct udev_device* padre, const char* subsistema){
	struct udev_device* hijo = NULL;
	struct udev_enumerate *enumerar = udev_enumerate_new(udev);

	udev_enumerate_add_match_parent(enumerar, padre);
	udev_enumerate_add_match_subsystem(enumerar, subsistema);
	udev_enumerate_scan_devices(enumerar);

	struct udev_list_entry *dispositivos = udev_enumerate_get_list_entry(enumerar);
	struct udev_list_entry *entrada;
	udev_list_entry_foreach(entrada, dispositivos){
		const char *ruta = udev_list_entry_get_name(entrada);
		hijo = udev_device_new_from_syspath(udev, ruta);
		break;
	}
	udev_enumerate_unref(enumerar); 
	return hijo;
}

/* esta funcion retorna la direccion logica del dispositivos que se encuentre conectado a la pc y cuyo
	nodo sea igual a direccion_fisica (argumento)*/
const char* direccionDispositivo(const char *direccion_fisica){
	FILE *fp;
	struct mntent *fs;
	/*function opens the filesystem description file filename and returns a file pointer*/
	fp = setmntent("/etc/mtab", "r");
	if (fp == NULL) {
		return "\"str_error\":\"ERROR: Al intentar abrir el fichero: /etc/mtab que contiene la direccion logico de los disp USB\"";
	}
	/* que leerá UNA linea del mtab, y les devolverá una estructura:*/
	while ((fs = getmntent(fp)) != NULL){
		/* resulta que direccion_fisica no contiene un numero al final que indica la particion correspondiente
		en caso de solo poseer una sola particion posee el numero 1 (esto es lo mas comun para un dispositivo usb)*/
		if(strstr(fs->mnt_fsname,direccion_fisica)>0){
			endmntent(fp);
			return fs->mnt_dir;
		}
	}
	endmntent(fp);
	return  "no se encuentra montado dicho dispositivo";
}

char* enumerar_disp_alm_masivo(struct udev* udev){

	struct udev_enumerate* enumerar = udev_enumerate_new(udev);

	//Buscamos los dispositivos USB del tipo SCSI (MASS STORAGE)
	udev_enumerate_add_match_subsystem(enumerar, "scsi");
	udev_enumerate_add_match_property(enumerar, "DEVTYPE", "scsi_device");
	udev_enumerate_scan_devices(enumerar);
	
	//Obtenemos los dispositivos con dichas caracteristicas
	struct udev_list_entry *dispositivos = udev_enumerate_get_list_entry(enumerar);
	struct udev_list_entry *entrada;

	//Recorremos la lista obtenida
	
	char *lista = (char *)malloc(BUFFDISPOSITIVOS);
	int n=0;
	udev_list_entry_foreach(entrada, dispositivos) {
		char *concat_str = (char *)malloc(BUFFDISPOSITIVOS);
		const char* ruta = udev_list_entry_get_name(entrada);
		struct udev_device* scsi = udev_device_new_from_syspath(udev, ruta);
		
		//obtenemos la información pertinente del dispositivo
		struct udev_device* block = obtener_hijo(udev, scsi, "block");
		struct udev_device* scsi_disk = obtener_hijo(udev, scsi, "scsi_disk");

		struct udev_device* usb= udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");
		
		if (block && scsi_disk && usb){
			const char *nodo=udev_device_get_devnode(block);
			const char * validarerror=direccionDispositivo(nodo);
			if(strstr(validarerror, "str_error")!=NULL ){
				return (char *)validarerror;
			}
			n=sprintf(concat_str, "{\"nodo\":\"%s\", \"nombre\":\" \",\"montaje\":\"%s\",\"Vendor:idProduct\":\"%s:%s\",\"scsi\":\"%s\"}\n", 
				nodo,
				direccionDispositivo(nodo),
				udev_device_get_sysattr_value(usb, "idVendor"),
				udev_device_get_sysattr_value(usb, "idProduct"),
				udev_device_get_sysattr_value(scsi, "vendor"));
			if(strstr(lista, "nodo")!=NULL){
				char *copia = (char *)malloc(BUFFDISPOSITIVOS);
				sprintf(copia, "%s",lista);
				sprintf(lista, "%s,%s",copia,concat_str);
			}else{
				sprintf(lista, "%s",concat_str);
			}
		}
		if (block) udev_device_unref(block);
		if (scsi_disk) udev_device_unref(scsi_disk);
		udev_device_unref(scsi);
		//validar para mas de dos dispositivos con contatenacion
		
	//		concat_str=NULL;
	}
	if(n==0) lista=" ";
	udev_enumerate_unref(enumerar);
	return lista;

}
