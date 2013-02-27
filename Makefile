# Makefile for the 3dsLoader app


CXX = g++
APP = 3dsloader
SRC = 3dsloader.cpp
OBJ = $(SRC:.cpp=.o)

all: ${APP}

$(APP) : $(OBJ)
	$(CXX) -o $@ $^

$(OBJ) : $(SRC)