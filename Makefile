GCC_FLAGS = -c #genera i file oggetto
INC_DIR = -I inc/ #flag -I serve per definire i percorsi dove sono contenuti gli header (file.h)
DEBUG = -g #flag per abilitare il debugging

all:	bin/F4Client

bin/F4Client:	bin/F4Server
	gcc obj/F4Client.o obj/semaphore.o obj/errExit.o obj/shared_memory.o  obj/matrix.o  obj/controllo.o obj/segnale.o -o bin/F4Client

bin/F4Server:	obj/F4Server.o obj/F4Client.o obj/semaphore.o obj/errExit.o obj/shared_memory.o obj/matrix.o obj/controllo.o obj/segnale.o
	gcc obj/F4Server.o obj/semaphore.o obj/errExit.o obj/shared_memory.o obj/matrix.o obj/controllo.o obj/segnale.o -o bin/F4Server

obj/F4Server.o: src/F4Server.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/F4Server.c -o obj/F4Server.o

obj/F4Client.o: src/F4Client.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/F4Client.c -o obj/F4Client.o

obj/semaphore.o: src/semaphore.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/semaphore.c -o obj/semaphore.o

obj/shared_memory.o: src/shared_memory.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/shared_memory.c -o obj/shared_memory.o

obj/errExit.o: src/errExit.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/errExit.c -o obj/errExit.o

obj/matrix.o: src/matrix.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/matrix.c -o obj/matrix.o

obj/controllo.o: src/controllo.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/controllo.c -o obj/controllo.o

obj/segnale.o: src/segnale.c 
	gcc $(GCC_FLAGS) $(DEBUG) $(INC_DIR) src/segnale.c -o obj/segnale.o

clean:
	rm -f obj/*.o bin/*