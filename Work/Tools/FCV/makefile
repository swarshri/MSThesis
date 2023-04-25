COM := ./../../Common
EXT := ./../../External
BWA := ./../../External/BWA
BWASRC := ./../../External/BWA
BWAMEM := ./../../External/BWA-MEM2
BWAMEMSRC := ./../External/BWA-MEM2/src

CFLAGS := -g -Wall -Wno-unused-function -O2 -fPIC
DFLAGS := -DHAVE_PTHREAD $(WRAP_MALLOC)
CXX := g++
CPPFLAGS := -fPIC -Wall -Wextra -g -O3 -std=c++11 -L $(BWA)

OBJS = ./fcv.o $(COM)/dip.o $(EXT)/libbwa.so

.PHONY: all clean

all: clean config

tool: $(OBJS)
	$(CXX) $(CPPFLAGS) -o ./fcvtool $(OBJS)

./fcv.o:
	$(CXX) $(CPPFLAGS) -c ./FCVTool.cpp -o ./fcv.o

$(COM)/dip.o:
	make dip.o -C $(COM)

$(BWA)/%.o: $(BWA)/%.c
	$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@

cleanC:
	make clean -C $(COM)

cleanO:
	rm -r *.o

cleanR:
	rm -r fcvtool

cleanOR: cleanO cleanR

cleanL: cleanO cleanR cleanD

clean: cleanC cleanO cleanR