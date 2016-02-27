CPP=g++
CC=gcc
POCOLIB=-lPocoFoundation -lPocoUtil
CFLAGS=-O3 -Wpedantic -std=c++11 -ldl -lpthread $(POCOLIB)

all: test cmodules slag

doc: doxy/config
	doxygen doxy/config 

dir: 
	mkdir -p build

test: dir AsyncQueueTest/AsyncQueueTest.cpp
	$(CPP) AsyncQueueTest/AsyncQueueTest.cpp $(CFLAGS) -o build/AsyncQueueTest

slag: dir Slag/*.cpp Slag/UNIX/*.cpp
	$(CPP) Slag/*.cpp Slag/UNIX/*.cpp $(CFLAGS) -I"inc" -I"Slag" -o build/Slag

cmodules: dir CModules/CModules.c
	$(CC) CModules/CModules.c -I"inc" -fPIC -shared -o build/CModules.so

clean:
	rm -R -f build/
