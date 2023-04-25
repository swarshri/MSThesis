SRC := ./src
INC := ./inc
EXT := ./../../External
BWA := ./../../External/BWA
BWASRC := ./../../External/BWA
BWAMEM := ./../../External/BWA-MEM2
BWAMEMSRC := ./../External/BWA-MEM2/src

BIN := ./bin
RLS := ./bin/release
DBG := ./bin/debug
OBJ := $(BIN)/obj
CC  := gcc
CFLAGS := -g -Wall -Wno-unused-function -O2 -fPIC
DFLAGS := -DHAVE_PTHREAD $(WRAP_MALLOC)
CXX := LD_LIBRARY_PATH=$(BWA) g++
CPPFLAGS := -fPIC -Wall -Wextra -g -O3 -std=c++11 -I $(EXT) -L $(BWA)

LIBS = bwa
OBJS = ./fcv.o -lz -lm -lpthread $(EXT)/libbwa.so
ELIBS =	-lm -lz -lpthread
LOBJS = $(BWA)/utils.o $(BWA)/kthread.o $(BWA)/kstring.o $(BWA)/ksw.o $(BWA)/bwt.o $(BWA)/bntseq.o $(BWA)/bwa.o $(BWA)/bwamem.o $(BWA)/bwamem_pair.o \
		$(BWA)/bwamem_extra.o $(BWA)/malloc_wrap.o $(BWA)/QSufSort.o $(BWA)/bwt_gen.o $(BWA)/rope.o $(BWA)/rle.o $(BWA)/is.o $(BWA)/bwtindex.o
AOBJS =	$(BWA)/bwashm.o $(BWA)/bwase.o $(BWA)/bwaseqio.o $(BWA)/bwtgap.o $(BWA)/bwtaln.o $(BWA)/bamlite.o \
		$(BWA)/bwape.o $(BWA)/kopen.o $(BWA)/pemerge.o $(BWA)/maxk.o \
		$(BWA)/bwtsw2_core.o $(BWA)/bwtsw2_main.o $(BWA)/bwtsw2_aux.o $(BWA)/bwt_lite.o \
		$(BWA)/bwtsw2_chain.o $(BWA)/fastmap.o $(BWA)/bwtsw2_pair.o

.PHONY: all clean

all: clean config

dbug: $(OBJS)
	$(CXX) $(CPPFLAGS) -o ./fcvtool_dbg $(OBJS)

tool: $(OBJS)
	$(CXX) $(CPPFLAGS) -o ./fcvtool $(OBJS)

./fcv.o:
	$(CXX) $(CPPFLAGS) -c ./FCVTool.cpp -o ./fcv.o

$(EXT)/libbwa.so: $(LOBJS) $(AOBJS)
	$(CC) -fPIC $(LOBJS) $(ELIBS) -shared -o $@

$(BWA)/%.o: $(BWA)/%.c
	$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@

cleanE:
	make clean -C $(BWA)
	rm -r $(EXT)/libbwa.so

cleanO:
	rm -r *.o

cleanR:
	rm -r fcvtool

cleanD:
	rm -r fcvtool_dbg

cleanOD: cleanO cleanD

cleanOR: cleanO cleanR

cleanL: cleanO cleanR cleanD

clean: cleanE cleanO cleanR