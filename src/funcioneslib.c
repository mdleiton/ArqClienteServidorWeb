#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <mntent.h>
#include <errno.h>
#include <string.h>

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


void enumerar_disp_alm_masivo(struct udev* udev,int logsdaemon){

	struct udev_enumerate* enumerar = udev_enumerate_new(udev);

	//Buscamos los dispositivos USB del tipo SCSI (MASS STORAGE)
	udev_enumerate_add_match_subsystem(enumerar, "scsi");
	udev_enumerate_add_match_property(enumerar, "DEVTYPE", "scsi_device");
	udev_enumerate_scan_devices(enumerar);
	
	//Obtenemos los dispositivos con dichas caracteristicas
	struct udev_list_entry *dispositivos = udev_enumerate_get_list_entry(enumerar);
	struct udev_list_entry *entrada;

	//Recorremos la lista obtenida
	char *concat_str = (char *)malloc(200);
	int n=0;
	udev_list_entry_foreach(entrada, dispositivos) {
		const char* ruta = udev_list_entry_get_name(entrada);
		struct udev_device* scsi = udev_device_new_from_syspath(udev, ruta);
		
		//obtenemos la información pertinente del dispositivo
		struct udev_device* block = obtener_hijo(udev, scsi, "block");
		struct udev_device* scsi_disk = obtener_hijo(udev, scsi, "scsi_disk");

		struct udev_device* usb
			= udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

		if (block && scsi_disk && usb) {
			n=sprintf(concat_str, "block = %s, usb = %s:%s, scsi = %s\n", 
				udev_device_get_devnode(block),
				udev_device_get_sysattr_value(usb, "idVendor"),
				udev_device_get_sysattr_value(usb, "idProduct"),
				udev_device_get_sysattr_value(scsi, "vendor"));
			//fwrite(concat_str, CONCAT_LEN, 1, fptr);
			write (logsdaemon, concat_str, n);
		}
		if (block) {
			udev_device_unref(block);
		}

		if (scsi_disk) {
			udev_device_unref(scsi_disk);
		}
		udev_device_unref(scsi);
	}
	if(n==0){
			write (logsdaemon, "no hay dispositivo conectado \n", 30);
		}
	udev_enumerate_unref(enumerar);
}

void presentar_estructuraMNTENT(const struct mntent *fs){
	printf("nodo :%s \n direccion logica :%s \n %s \n %s \n %d \n %d\n",
		fs->mnt_fsname,  /* name of mounted filesystem(es el nodo del dispositivo) */
		fs->mnt_dir,    /* filesystem path prefix (el directorio donde está montado.)*/
		fs->mnt_type,	/* mount type  */
		fs->mnt_opts,	/* mount options  */
		fs->mnt_freq,	/* dump frequency in days */
		fs->mnt_passno);	/* pass number on parallel fsck */
}

void direccionDispositivo(const char *filename,const char *direccion_fisica){
	FILE *fp;
	struct mntent *fs;
	/*function opens the filesystem description file filename and returns a file pointer*/
	fp = setmntent(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, " %s: error al intentar abrir el archivo: %s\n", filename, strerror(errno));
		exit(1);
	}
	/* que leerá UNA linea del mtab, y les devolverá una estructura:*/
	while ((fs = getmntent(fp)) != NULL){
		if(strcmp(fs->mnt_fsname,direccion_fisica)==0){
			presentar_estructuraMNTENT(fs);
		}
	}
	endmntent(fp);
}
