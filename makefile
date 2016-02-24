all: compile link

compile: main.cpp
	clang++-3.7 -std=c++11 -c main.cpp

link: main.o
	clang++-3.7 -std=c++11 main.o -o main -lsfml-graphics -lsfml-window -lsfml-system
