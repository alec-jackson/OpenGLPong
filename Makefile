# OpenGL Makefile
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	LFLAGS = -lGLEW -lSDL2 -lSDL2_mixer -lSDL2_image -lGL -lOpenGL -pthread
	CFLAGS = -std=c++11 -g
endif
ifeq ($(UNAME_S),Darwin)
	LFLAGS = -lSDL2 -lSDL2_mixer -lSDL2_image -framework OpenGL -lglew
	CFLAGS = -Wno-deprecated-declarations -std=c++11
endif

#build instructions

pong: pong.o common/loadshaders.o
	g++ $(CFLAGS) -o pong pong.o common/loadshaders.o $(LFLAGS)

pong.o: pong.cpp
	g++ $(CFLAGS) -c pong.cpp

common/loadshaders.o: common/loadshaders.cpp
	g++ $(CFLAGS) -c common/loadshaders.cpp
	mv loadshaders.o common/loadshaders.o

clean:
	rm -r pong pong.o common/loadshaders.o

#dependency list
common/loadshader.cpp pong.cpp: common/loadshaders.h
