all: main

main: main.cpp function.h proxy.h logger.h exception.h parserRequest.h
	g++ -g  -o main main.cpp function.cpp proxy.cpp parserRequest.cpp   -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main