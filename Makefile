CC=g++
CF= -std=c++11

all: httpd

httpd: httpd.o fdset.o socket.o utils.o request.o response.o
	$(CC) $(CF) -o httpd httpd.o fdset.o socket.o utils.o request.o response.o

httpd.o : httpd.cc
	$(CC) $(CF) -c -I ./ httpd.cc

fdset.o : fdset.cc fdset.h
	$(CC) $(CF) -c -I ./ fdset.cc

socket.o : socket.cc socket.h
	$(CC) $(CF) -c -I ./ socket.cc

utils.o : utils.cc utils.h
	$(CC) $(CF) -c -I ./ utils.cc

request.o : request.cc request.h
	$(CC) $(CF) -c -I ./ request.cc

response.o : response.cc response.h
	$(CC) $(CF) -c -I ./ response.cc

clean:
	rm -f httpd.o fdset.o socket.o utils.o request.o response.o httpd
