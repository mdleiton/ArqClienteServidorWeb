ArqClienteServidorWeb 
======================

Version 1.0 15/08/2017 

1 - Descripcion
---------------
Este programa permite monitoriar las puertos USB de nuestra computadora, detectar la conexion de nuevos dispositivos USB a traves de un proceso(daemon). 
Un servidor web solicitará dicha informacion a este proceso para responder a las solicitudes realizadas por un cliente (escrito en python).

El cliente será capaz de solicitar :

* Una lista detallada de todos los dispositivos conectados a la computadora.
* Escribir archivos enviados por el cliente en cualquier dispositivo USB conectado a la computadora .
* Leer archivo de cualquier dispositivo USB conectado a la computadora y enviarlo al cliente que solicite dicho archivo.



2 - Instalacion
----------------
Para en funcionamiento de este programa es necesario contar con las siguientes librerias:

* Libreria libudev
* Libreria libmicrohttpd
* Libreria 

```
sudo apt-get install libudev-dev
sudo apt-get install libmicrohttpd
```

3 - Uso general
----------------





4 - Autores
-----------

* Leiton Mauricio
* Wong Hugo
* Zambrano Jhonny
