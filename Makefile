procesodaemon: funcioneslib.o daemonUSB.o
	gcc -Wall -Iinclude/ obj/*[!1].o -ludev -o bin/programa

funcioneslib.o: src/funcioneslib.c
	gcc -Wall -c  -Iinclude/ -ludev src/funcioneslib.c -o obj/funcioneslib.o

daemonUSB.o: src/daemonUSB.c
	gcc -Wall -c  -ludev  -pthread -Iinclude/  src/daemonUSB.c -o obj/daemonUSB.o

ServidorWeb: src/ServidorWeb.c
