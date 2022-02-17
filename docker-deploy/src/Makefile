all: main

main: main.cpp function.h proxy.h logger.h exception.h parserRequest.h parserResponse.h
	g++ -g  -o main main.cpp function.cpp proxy.cpp parserRequest.cpp parserResponse.cpp   -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main