ArqClienteServidorWeb 
======================

Version 1.0 15/08/2017 

1 - Descripción
---------------
Este programa permite monitoriar los puertos USB de nuestra computadora, detectar la conexión de nuevos dispositivos USB a traves de un proceso(daemon). 
Un servidor web solicitará dicha información a este proceso para responder a las solicitudes realizadas por un cliente (escrito en python).

El cliente será capaz de solicitar :

* Solicitar una lista detallada de todos los dispositivos conectados a la computadora.
* Nombrar a cualquier dispositivos que se encuentre conectado en la computadora.
* Escribir archivos enviados por el cliente en cualquier dispositivo USB conectado a la computadora .
* Leer archivo de cualquier dispositivo USB conectado a la computadora y enviarlo al cliente que solicite dicho archivo.

2 - Instalación
----------------
Para en funcionamiento de este programa es necesario contar con las siguientes librerias:

* Libreria libudev
* Libreria libmicrohttpd

```
sudo apt-get install libudev-dev
sudo apt-get install libmicrohttpd*
```

Para crear los ejecutables se disponible de una archivo makefile que le facilitará el trabajo.
para la crear el ejecutable del daemonUSB y el servidor web.

```
make
```

3 - Modo de uso general
------------------------

* iniciar USB-daemon.

El daemon por defecto escuchará las solicitudes servidor web por el puerto número 8888
Este proceso se encargará de responder las solicitudes del servidor web.

```
./bin/USB-daemon
```	

* iniciar servidor web.

El servidor web actuará de intermediara entre el cliente y el proceso daemon.
Recibe la solicitud del cliente, la procesa y envia la solicutud al proceso daemon, luego recibe respuesta del daemon de la solicitud realiza, la procesa y la envia al cliente. Al ejecutarlo se debe enviar de parametro el puerto . debe ser diferente al puerto 8888 con el cual esta escuchando el procesodaemon.

```
./bin/ServidorWeb 8889
```	
No cerrar esta terminal.

Una vez realizado esto. Se pueden realizar solicitudes desde el cliente python.

Formato general solicitud.
El puerto debe ser mismo que se agregó de parametro al ejecutar el servidor web.

```
python ./bin/cliente.py <puerto| 8889> <metodo> <tipo solicitud> <arg 1> <arg 2>.....<arg n>
```	


* Listar dispositivos
Permite obtener una lista detallada de todos los dispositivos USB conectados
```
python ./src/cliente.py <puerto|8889> GET listar_dispositivos
```	
ejemplo:
```
python ./src/cliente.py 8889 GET listar_dispositivos
```	

* Nombrar dispositivo
Permite asignar un nombre a un particular dispositivo USB para manejarlo de una mejor manera en futuras solicitudes. 
```
python ./src/cliente.py <puerto|8889> POST nombrar_dispositivo <nodo> <nombre>
```	
ejemplo:
```
python ./src/cliente.py 8889 POST nombrar_dispositivo  /dev/sda1 david21
```	
Asumiendo que existe un dispositivo en /dev/sda1 y que se encuentra en la lista obtenida en la anterior solicitud.


* Leer archivo.
Permite obtener el contenido de un archivo/fichero alojado en un dispositivo USB nombrado previamente. 
```
python ./src/cliente.py <puerto|8889> GET leer_archivo <nombre usb> <nombre archivo>
```	
ejemplo:
```
python ./src/cliente.py 8889 GET leer_archivo david21 comoserproenC.txt
```	


* Escribir archivo. 
Permite crear y escribir un archivo/fichero en un dispositivo USB nombrado previamente. Se debe especifica el fichero que se quiere escribir.
```
python ./src/cliente.py <puerto|8889> POST escribir_archivo <nombre usb> <nombre archivo>
```	
ejemplo:
```
python ./src/cliente.py 8889 POST escribir_archivo david21 programador.txt
```	

Adicional a esto existe un script clientep.py que permite verificar otro tipos de solicitudes y sus respectivas validacion de parte del servidor.En caso que lo requieran.


4 - Autores
-----------

* Leiton Mauricio
* Wong Hugo
* Zambrano Jhonny
