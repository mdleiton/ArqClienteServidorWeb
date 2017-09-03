struct udev_device* obtener_hijo(struct udev* udev, struct udev_device* padre, const char* subsistema);
char * enumerar_disp_alm_masivo(struct udev* udev);
void presentar_estructuraMNTENT(const struct mntent *fs);
void direccionDispositivo(const char *direccion_fisica);
void escuchandoSolicitudesClientes();
char* Dispositivo(char *direccion_fisica);
