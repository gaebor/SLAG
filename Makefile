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

dummy_build_folder := $(shell mkdir -p $(OUT_DIR))

all: cmodules mymodules slag

doc: doxy/config
	doxygen doxy/config 

Slag/%.o:Slag/%.cpp
	g++ -c $(CPPFLAGS) -I$(ASYNCQUEUE_DIR)/inc $< -o $@

$(OUT_DIR)/Slag: $(SLAG_OBJ)
	g++ $(CPPFLAGS) $(SLAG_OBJ) -L$(ASYNCQUEUE_DIR)/bin -lasyncqueue -ldl -o $@

slag: $(OUT_DIR)/Slag
	
CModules/CModules.o: CModules/CModules.c
	gcc -c $(CFLAGS) $(SHARED) $^ -o $@

$(OUT_DIR)/CModules.so: CModules/CModules.o
	gcc $(CFLAGS) $(SHARED) CModules/CModules.o -o $@

cmodules: $(OUT_DIR)/CModules.so

MYMODULES_SRC=MyModules/AbstractInterface.cpp\
    MyModules/AddModule.cpp\
    MyModules/KeyReader.cpp\
    MyModules/MyModules.cpp\
    MyModules/ImageProcessor.cpp\
    MyModules/VideoSource.cpp

MYMODULES_OBJ=$(MYMODULES_SRC:.cpp=.o)

MyModules/%.o: MyModules/%.cpp
	g++ -c $(CPPFLAGS) $(SHARED) $< -o $@
    
$(OUT_DIR)/MyModules.so: $(MYMODULES_OBJ)
	g++ $(CPPFLAGS) $(SHARED) $(MYMODULES_OBJ) $(OPENCVLIB) -o $@

mymodules: $(OUT_DIR)/MyModules.so

clean:
	rm -Rf $(OUT_DIR)/ Slag/*.o Slag/UNIX/*.o CModules/*.o MyModules/*.o
