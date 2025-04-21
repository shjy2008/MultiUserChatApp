include ../Make.defines

PROGS = chatserver chatclient

all:	${PROGS}

chatserver:	        chatserver.o
		            ${CC} ${CFLAGS} -o $@ chatserver.o ${LIBS}

chatclient:	        chatclient.o
		            ${CC} ${CFLAGS} -o $@ chatclient.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
