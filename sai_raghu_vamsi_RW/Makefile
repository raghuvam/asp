
PWD=$(shell pwd)

all: messaging writer reader
# -Wall -Werror -Wall -Wall
messaging:
	gcc -std=c89 -lrt -c -fpic src/messaging.c 
	mkdir -p libs 
	gcc -shared -o libs/libmessaging.so messaging.o
	rm messaging.o

writer:
	g++ -std=c++03 -L$(PWD)/libs  -o writer src/writer.cpp  -lmessaging -lrt

reader:
	g++ -std=c++03 -L$(PWD)/libs -o reader src/reader.cpp -lmessaging -lrt

clean:
	rm -f libs/libmessaging.so messaging.o reader writer
	rm -r libs

