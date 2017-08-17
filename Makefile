INCLUDE=-Iinclude/
LIBS=-Llib/
programa: enumerar_disp_alm_masivo.o src/daemonUSB.c src/ServidorWeb.c
	gcc -Wall -c  -ludev  -pthread -Iinclude/  src/daemonUSB.c -o obj/daemonUSB.o
	gcc -Wall -c -I$PATH_TO_LIBMHD_INCLUDES -L$PATH_TO_LIBMHD_LIBS -lmicrohttpd -pthread src/ServidorWeb.c -o obj/ServidorWeb1.o
	gcc -Wall -Iinclude/ obj/*[!1].o -ludev -o bin/programa

enumerar_disp_alm_masivo.o: src/enumerar_disp_alm_masivo.c
	gcc -Wall -c  -Iinclude/ -ludev src/enumerar_disp_alm_masivo.c -o obj/enumerar_disp_alm_masivo.o


.PHONY: clean
clean:
	rm -rf obj/* bin/* lib/*
