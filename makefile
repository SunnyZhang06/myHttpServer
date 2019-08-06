all:server

server:main.o event.o
	g++ main.o event.o -o server

mian.o:main.cpp
	g++ -c main.cpp

event.o:event.cpp
	g++ -c event.cpp
	
.PHONY:clean

clean:
	rm -rf *o server

