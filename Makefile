OPENCVLIB=-lopencv_core -lopencv_imgproc -lopencv_highgui
CPPFLAGS=-O3 -Wpedantic -std=c++11 -Iinc -pthread
CFLAGS=-O3 -Wpedantic -Iinc
SHARED=-fPIC -shared
ASYNCQUEUE_DIR=../AsyncQueue
OUT_DIR=bin

SLAG_SRC=Slag/ConfigReader.cpp\
Slag/Factory.cpp\
Slag/HumanReadable.cpp\
Slag/InternalTypes.cpp\
Slag/ModuleIdentifier.cpp\
Slag/ModuleWrapper.cpp\
Slag/OutputText.cpp\
Slag/Slag.cpp\
Slag/UNIX/Imshow.cpp\
Slag/UNIX/LoadLibrary.cpp\
Slag/UNIX/TerminationSignal.cpp

SLAG_OBJ=$(SLAG_SRC:.cpp=.o)

all: cmodules mymodules slag

doc: doxy/config
	doxygen doxy/config 

dir:
	mkdir -p $(OUT_DIR)

Slag/%.o:Slag/%.cpp
	g++ -c $(CPPFLAGS) -I$(ASYNCQUEUE_DIR)/inc $< -o $@

slag: dir $(SLAG_OBJ)
	ld $(SLAG_OBJ) -L$(ASYNCQUEUE_DIR)/bin -lasyncqueue -ldl -lpthread -o $(OUT_DIR)/Slag

cmodules: dir CModules/CModules.c
	gcc CModules/CModules.c $(CFLAGS) $(SHARED) -o $(OUT_DIR)/CModules.so

mymodules: dir MyModules/*.cpp
	g++ MyModules/*.cpp $(CPPFLAGS) $(OPENCVLIB) $(SHARED) -o $(OUT_DIR)/MyModules.so

clean:
	rm -R -f $(OUT_DIR)/ Slag/*.o Slag/UNIX/*.o
