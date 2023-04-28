SRC := ./src
INC := ./inc
COM := ./../Common
EXT := ./../External
BWA := $(EXT)/BWA
BWASRC := $(EXT)/BWA
BWAMEM := $(EXT)/BWA-MEM2
BWAMEMSRC := $(EXT)/BWA-MEM2/src
DRAMSIM3 := $(EXT)/DRAMsim3
DRAMSIM3_SRC := $(EXT)/DRAMsim3/src
FMT_LIB := $(EXT)/DRAMsim3/ext/fmt/include
INI_LIB := $(EXT)/DRAMsim3/ext/headers

BIN := ./bin
OBJ := $(BIN)/obj

CC := gcc
CFLAGS := -g -Wall -Wno-unused-function -O2 -fPIC
DFLAGS := -DHAVE_PTHREAD $(WRAP_MALLOC)
CXX = g++
CPPFLAGS := -fPIC -Wall -Wextra -g -O3 -std=c++11 -I $(INC) -I $(EXT) -I $(FMT_LIB) -I $(INI_LIB) -L $(DRAMSIM3) -L $(EXT)

LIBS = dramsim3
OBJS = $(OBJ)/main.o $(OBJ)/cache.o $(OBJ)/compute.o $(OBJ)/config.o $(OBJ)/core.o $(OBJ)/dram.o $(OBJ)/dispatch.o $(OBJ)/fetch.o $(OBJ)/load.o \
	   $(OBJ)/perfrec.o $(OBJ)/queue.o $(OBJ)/rs.o $(OBJ)/reserve.o $(OBJ)/store.o $(COM)/dip.o $(EXT)/libbwa.so $(DRAMSIM3)/libdramsim3.so

.PHONY: all clean

all: clean model

model: $(OBJS)
	$(CXX) $(CPPFLAGS) -o ./model $(OBJS)

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

$(COM)/dip.o:
	make -C $(COM)

$(DRAMSIM3)/libdramsim3.so:
	make -C $(DRAMSIM3)

cleanE:
	make clean -C $(COM)
	make clean -C $(DRAMSIM3)

cleanO:
	rm -r $(OBJ)
	mkdir $(OBJ)

cleanR:
	rm -f ./model

cleanOD: cleanO cleanD

cleanOR: cleanO cleanR

clean: cleanE cleanO cleanR