INCLUDE=-Iinclude/
LIBS=-Llib/
programa: ./src/daemonUSB.c ./src/ServidorWeb.c
	gcc -Wall -pthread ./src/daemonUSB.c -o ./bin/daemonUSB
	gcc -Wall -pthread ./src/ServidorWeb.c -o ./bin/ServidorWeb


.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
