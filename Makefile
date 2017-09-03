INCLUDE=-Iinclude/
LIBS=-Llib/
procesodaemon: funcioneslib.o daemonUSB.o ServidorWeb
	gcc -Wall -Iinclude/ obj/*[!1].o -ludev -o bin/USB-daemon

funcioneslib.o: src/funcioneslib.c
	gcc -Wall -c  -Iinclude/ -ludev src/servidor_daemon.c -o obj/funciones1lib.o
	gcc -Wall -c  -Iinclude/ -ludev src/funcioneslib.c -o obj/funcioneslib.o


daemonUSB.o: src/daemonUSB.c
	gcc -Wall -c  -ludev  -pthread -Iinclude/  src/daemonUSB.c -o obj/daemonUSB.o

ServidorWeb: src/ServidorWeb.c
	gcc src/ServidorWeb.c -o bin/ServidorWeb -Llib/ -I$PATH_TO_LIBMHD_INCLUDES -L$PATH_TO_LIBMHD_LIBS -lmicrohttpd -Iinclude/ -ljsmn
	
.PHONY: clean
clean:
	rm -rf obj/* bin/*  
