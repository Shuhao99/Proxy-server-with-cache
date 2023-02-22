all: proxy

proxy: main.cpp proxy_server.cpp request.cpp response.cpp socket.cpp session.h
	g++ -g -o main main.cpp proxy_server.cpp request.cpp response.cpp socket.cpp session.h -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
