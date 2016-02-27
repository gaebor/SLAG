CPP=g++
CC=gcc
OPENCVLIB=-lopencv_core -lopencv_imgproc
POCOLIB=-lPocoFoundation -lPocoUtil
CPPFLAGS=-O3 -Wpedantic -std=c++11 -ldl -lpthread
CFLAGS=-O3 -Wpedantic
SHARED=-fPIC -shared

all: test cmodules mymodules slag

doc: doxy/config
	doxygen doxy/config 

dir: 
	mkdir -p build

test: dir AsyncQueueTest/AsyncQueueTest.cpp
	$(CPP) AsyncQueueTest/AsyncQueueTest.cpp $(CPPFLAGS) $(POCOLIB) -o build/AsyncQueueTest

slag: dir Slag/*.cpp Slag/UNIX/*.cpp
	$(CPP) Slag/*.cpp Slag/UNIX/*.cpp $(CPPFLAGS) $(POCOLIB) -I"inc" -I"Slag" -o build/Slag

cmodules: dir CModules/CModules.c
	$(CC) CModules/CModules.c -I"inc" $(SHARED) -o build/CModules.so

mymodules: dir MyModules/*.cpp
	$(CPP) MyModules/*.cpp $(CPPFLAGS) -I"inc" -I"MyModules" $(OPENCVLIB) $(SHARED) -o build/MyModules.so
clean:
	rm -R -f build/
