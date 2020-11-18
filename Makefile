# OpenGL Makefile
UNAME_S = $(shell uname -s) #Get system name and use this to decide what flags to use
ifeq ($(UNAME_S),Linux )
	LFLAGS = -lGLEW -lSDL2 -lSDL2_mixer -lGL -lOpenGL -pthread
	CFLAGS = -std=c++11 -g
endif
ifeq ($(UNAME_S),Darwin)
	LFLAGS = -framework SDL2 -framework SDL2_mixer -framework OpenGL -lglew -F libs -rpath libs
	CFLAGS = -Wno-deprecated-declarations -std=c++11
endif

#build instructions

pong: pong.o common/loadshaders.o
	g++ $(CFLAGS) -o pong pong.o common/loadshaders.o $(LFLAGS)

pong.o: pong.cpp
	g++ $(CFLAGS) -c -o pong.o pong.cpp -lSDL2 -lGLEW -lSDL2 -lSDL2_mixer -lGL -lOpenGL -pthread

common/loadshaders.o: common/loadshaders.cpp
	g++ $(CFLAGS) -c -o common/loadshaders.o common/loadshaders.cpp

clean:
	rm -r pong pong.o common/loadshaders.o

#dependency list
common/shader.cpp pong.cpp: common/loadshaders.h
