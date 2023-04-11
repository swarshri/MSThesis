SRC := ./src
INC := ./inc
EXT := ./ext
DRAMSIM3 := ./ext/DRAMsim3
DRAMSIM3_SRC := ./ext/DRAMsim3/src
FMT_LIB := ./ext/DRAMsim3/ext/fmt/include
INI_LIB := ./ext/DRAMsim3/ext/headers

BIN := ./bin
RLS := ./bin/release
DBG := ./bin/debug
OBJ := $(BIN)/obj
CXX = g++
CPPFLAGS := -fPIC -Wall -Wextra -g -O3 -std=c++11 -I $(INC) -I $(EXT) -I $(FMT_LIB) -I $(INI_LIB) -L $(DRAMSIM3)

LIBS = dramsim3
OBJS = $(OBJ)/main.o $(OBJ)/cache.o $(OBJ)/compute.o $(OBJ)/config.o $(OBJ)/core.o $(OBJ)/dram.o $(OBJ)/dispatch.o $(OBJ)/fetch.o $(OBJ)/load.o $(OBJ)/perfrec.o $(OBJ)/queue.o $(OBJ)/rs.o $(OBJ)/reserve.o $(OBJ)/store.o $(DRAMSIM3)/libdramsim3.so

.PHONY: all clean

all: clean config

dbug: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(DBG)/model $(OBJS)

model: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(RLS)/model $(OBJS)

$(OBJ)/main.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/main.cpp -o $(OBJ)/main.o

$(OBJ)/cache.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Cache.cpp -o $(OBJ)/cache.o

$(OBJ)/compute.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Compute.cpp -o $(OBJ)/compute.o

$(OBJ)/config.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Config.cpp -o $(OBJ)/config.o

$(OBJ)/core.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Core.cpp -o $(OBJ)/core.o

$(OBJ)/dispatch.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Dispatch.cpp -o $(OBJ)/dispatch.o

$(OBJ)/dram.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/DRAMWrapper.cpp -o $(OBJ)/dram.o

$(OBJ)/fetch.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Fetch.cpp -o $(OBJ)/fetch.o

$(OBJ)/load.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Load.cpp -o $(OBJ)/load.o

$(OBJ)/perfrec.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/PerfRecorder.cpp -o $(OBJ)/perfrec.o

$(OBJ)/queue.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Queue.cpp -o $(OBJ)/queue.o

$(OBJ)/rs.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/ReservationStation.cpp -o $(OBJ)/rs.o

$(OBJ)/reserve.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Reserve.cpp -o $(OBJ)/reserve.o

$(OBJ)/store.o:
	$(CXX) $(CPPFLAGS) -c $(SRC)/Store.cpp -o $(OBJ)/store.o

$(DRAMSIM3)/libdramsim3.so:
	make -C $(DRAMSIM3)

cleanE:
	make clean -C $(DRAMSIM3)

cleanO:
	rm -r $(OBJ)
	mkdir $(OBJ)

cleanR:
	rm -r $(RLS)
	mkdir $(RLS)

cleanD:
	rm -r $(DBG)
	mkdir $(DBG)

cleanOD: cleanO cleanD

cleanOR: cleanO cleanR

cleanL: cleanO cleanR cleanD

clean: cleanE cleanO cleanR cleanD