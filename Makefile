# OpenGL Makefile

CC = g++
CFLAGS = -g -Wall
LDLIBS=-lglut -lGLEW -lGL -lSDL2

pong: pong.o common/loadshaders.o
	g++ -Wall -g -o pong pong.o common/loadshaders.o  -lGLEW -lSDL2 -lSDL2_mixer -lGL -lOpenGL -pthread

pong.o: pong.cpp
	g++ -Wall -g -c -o pong.o pong.cpp -lSDL2 -lGLEW -lSDL2 -lSDL2_mixer -lGL -lOpenGL -pthread

common/loadshaders.o: common/loadshaders.cpp
	g++ -Wall -g -c -o common/loadshaders.o common/loadshaders.cpp

clean:
	rm -r pong pong.o

#dependency list
common/shader.cpp pong.cpp: common/loadshaders.h
