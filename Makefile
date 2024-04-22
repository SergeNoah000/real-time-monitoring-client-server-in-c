CC = gcc
CFLAGS = -Wall -Wextra -Wno-format-contains-nul 

.PHONY: all compil run c

all:runnerc




#compil:programme

runnerc: 
	$(CC) $(CFLAGS) -o server_runner  server/lib/functions.c server/server_main.c -lpthread --no-warning && $(CC) $(CFLAGS) -o client_watch client/lib/functions.c client/client_main.c  


clean:
	rm -f server_runner client_watch 

run:
	./server_runner 8080
