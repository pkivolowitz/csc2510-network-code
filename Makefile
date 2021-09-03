CC = g++
name = server

all:
	$(CC) -std=c++11 $(name).cpp -pthread -o $(name)