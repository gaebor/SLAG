CC=g++
POCOLIB=-lPocoFoundation -lPocoUtil
CFLAGS=-O3 -Wall -std=c++11 -lpthread $(POCOLIB) -I"AsyncQueueTest/"


doc: doxy/config
	doxygen doxy/config 

dir: 
	mkdir -p build

test: dir AsyncQueueTest/AsyncQueueTest.cpp
	$(CC) AsyncQueueTest/AsyncQueueTest.cpp $(CFLAGS) -o build/AsyncQueueTest

slag: dir 


clean:
	rm -R -f build/