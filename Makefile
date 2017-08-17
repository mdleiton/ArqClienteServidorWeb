INCLUDE=-Iinclude/
LIBS=-Llib/
programa: enumerar_disp_alm_masivo.o src/daemonUSB.c src/ServidorWeb.c
	gcc -Wall -c -ludev  -pthread -Iinclude/ src/daemonUSB.c -o obj/daemonUSB.o
	gcc -Wall -c -pthread src/ServidorWeb.c -o obj/ServidorWeb.o
	gcc -Wall -Iinclude/ obj/*.o -ludev -o bin/programa

enumerar_disp_alm_masivo.o: src/enumerar_disp_alm_masivo.c
	gcc -Wall -c  -Iinclude/ -ludev src/enumerar_disp_alm_masivo.c -o obj/enumerar_disp_alm_masivo.o


.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
