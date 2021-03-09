CFLAGS=-Wall -g
CTEST=-g -Wall -coverage -fpic -lm


all: enc_client enc_server dec_server dec_client keygen

enc_client: enc_client.c
	gcc $^ -std=gnu99 -o $@ ${CFLAGS}

enc_server: enc_server.c
	gcc $^ -std=gnu99 -o $@ ${CFLAGS}

dec_client: dec_client.c
	gcc $^ -std=gnu99 -o $@ ${CFLAGS}

dec_server: dec_server.c
	gcc: $^ -std=gnu99 -o $@ ${CFLAGS}

keygen: keygen.c
	gcc: $^ -std=gnu99 -o $@ ${CFLAGS}

clean:
	rm -f enc_client enc_sever dec_client dec_server keygen
