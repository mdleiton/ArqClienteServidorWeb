INCLUDE=-Iinclude/
LIBS=-Llib/
procesodaemon: funcioneslib.o daemonUSB.o
	gcc -Wall -Iinclude/ obj/*[!1].o -ludev -o bin/programa

funcioneslib.o: src/funcioneslib.c
	gcc -Wall -c  -Iinclude/ -ludev src/funcioneslib.c -o obj/funcioneslib.o

daemonUSB.o: src/daemonUSB.c
	gcc -Wall -c  -ludev  -pthread -Iinclude/  src/daemonUSB.c -o obj/daemonUSB.o

ServidorWeb.o: src/ServidorWeb.c
	gcc -Wall -c -I$PATH_TO_LIBMHD_INCLUDES -L$PATH_TO_LIBMHD_LIBS -lmicrohttpd -pthread src/ServidorWeb.c -o obj/ServidorWeb1.o

.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
