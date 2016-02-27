CC=g++
POCOLIB=-lPocoFoundation -lPocoUtil
CFLAGS=-O3 -Wall -std=c++11 -lpthread $(POCOLIB)

all: test cmodules slag

doc: doxy/config
	doxygen doxy/config 

dir: 
	mkdir -p build

test: dir AsyncQueueTest/AsyncQueueTest.cpp
	$(CC) AsyncQueueTest/AsyncQueueTest.cpp $(CFLAGS) -I"AsyncQueueTest/" -o build/AsyncQueueTest

slag: dir Slag/*.cpp
	$(CC) Slag/*.cpp $(CFLAGS) -I"Slag" -I"inc" -o build/Slag

cmodules: dir CModules/CModules.c
	gcc CModules/CModules.c -I"inc" -fPIC -shared -o build/CModules.so

clean:
	rm -R -f build/
