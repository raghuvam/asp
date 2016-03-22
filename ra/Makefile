
all: messaging writer reader
# -Wall -Werror -Wall -Wall
messaging:
	gcc -c  -fpic messaging.c
	gcc -shared -o libmessaging.so messaging.o

writer:
	gcc -L/home/rv/asp/ra -o writer writer.c -lmessaging 

reader:
	gcc -L/home/rv/asp/ra -o reader reader.c -lmessaging

clean:
	rm -f libmessaging.so messaging.so messaging.o reader writer

