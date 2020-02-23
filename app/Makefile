TARGET = server client server_ftp client_ftp
DEPS = message.h
CFLAGS = -lpthread


all:  ${TARGET}

server: server.c ${DEPS}
	gcc -o server server.c ${CFLAGS}

client: client.c ${DEPS}
	gcc -o client client.c ${CFLAGS}

server_ftp: server_ftp.c
	gcc -o server_ftp server_ftp.c 

client_ftp: deliver_ftp.c
	gcc -o client_ftp deliver_ftp.c 

clean:
	rm -f ${TARGET}
