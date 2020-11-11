# OpenGL Makefile

CC = g++
CFLAGS = -g -Wall
LDLIBS=-lglut -lGLEW -lGL -lSDL2

pong: pong.o common/loadshaders.o
	g++ -Wall -g -o pong pong.o common/loadshaders.o  -framework SDL2 -framework SDL2_mixer -framework OpenGL -lglew -F libs -rpath libs

pong.o: pong.cpp
	g++ -Wall -g -c -o pong.o pong.cpp -framework SDL2 -framework SDL2_mixer -framework OpenGL -lglew -F libs -rpath libs

common/loadshaders.o: common/loadshaders.cpp
	g++ -Wall -g -c -o common/loadshaders.o common/loadshaders.cpp

clean:
	rm -r pong pong.o

#dependency list
common/shader.cpp pong.cpp: common/loadshaders.h
