SRC := ./src
INC := ./inc
BIN := ./bin
RLS := ./bin/release
DBG := ./bin/debug
OBJ := $(BIN)/obj
CXX = g++
CPPFLAGS := -g -O3 -Wall -std=c++11 -I $(INC)

OBJS = $(OBJ)/main.o $(OBJ)/config.o $(OBJ)/dram.o

.PHONY: all clean

all: clean config

dbug: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(DBG)/model $(OBJS)

model: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(RLS)/model $(OBJS)

$(OBJ)/main.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/main.cpp -o $(OBJ)/main.o

$(OBJ)/config.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Config.cpp -o $(OBJ)/config.o

$(OBJ)/dram.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/DRAM.cpp -o $(OBJ)/dram.o