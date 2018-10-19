CPPFLAGS= -Wall -Wextra -pthread -g3 -std=c++11

SRCS=main.cpp node.cpp parser.cpp message.cpp client.cpp server.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

main: $(OBJS)
	g++ $(CPPFLAGS) -o main main.o node.o parser.o message.o client.o server.o 

main.o: main.cpp node.h client.h message.h parser.h server.h
	g++ $(CPPFLAGS) -c main.cpp 

node.o: node.cpp node.h 
	g++ $(CPPFLAGS) -c node.cpp

message.o: message.cpp message.h 
	g++ $(CPPFLAGS) -c message.cpp

parser.o: parser.cpp parser.h node.h
	g++ $(CPPFLAGS) -c parser.cpp

client.o: client.cpp client.h node.h message.h
	g++ $(CPPFLAGS) -c client.cpp

server.o: server.cpp server.h node.h client.h
	g++ $(CPPFLAGS) -c server.cpp


              
