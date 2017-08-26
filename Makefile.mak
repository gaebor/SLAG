###############
# dependecies #
###############
ASYNCQUEUE_DIR=..\AsyncQueue

OPENCV_DIR=E:\PROGRAMOK\opencv\build
OPENCV_BIN=$(OPENCV_DIR)\x64\vc14
OPENCV_VERSION=2413
###############

ASYNCQUEUE_LIB=$(ASYNCQUEUE_DIR)\bin\asyncqueue.lib

WINAPI_LIB=shlwapi.lib gdi32.lib user32.lib shell32.lib
 
OUT_DIR=bin

!if [if not exist $(OUT_DIR) mkdir $(OUT_DIR)]
!endif

CL_FLAGS=/MD /W4 /O2 /Ot /Qpar /GL /DNDEBUG /Iinc
CPP_FLAGS=/c /EHsc $(CL_FLAGS)
C_FLAGS=/TC $(CL_FLAGS)

SLAG_SRC=Slag/ConfigReader.cpp\
    Slag/Factory.cpp\
    Slag/HumanReadable.cpp\
    Slag/InternalTypes.cpp\
    Slag/ModuleIdentifier.cpp\
    Slag/ModuleWrapper.cpp\
    Slag/OutputText.cpp\
    Slag/Slag.cpp\
    Slag/WIN/Imshow.cpp\
    Slag/WIN/LoadLibrary.cpp\
    Slag/WIN/TerminationSignal.cpp

SLAG_OBJ=$(SLAG_SRC:cpp=obj)

MYMODULES_SRC=MyModules/AbstractInterface.cpp\
    MyModules/AddModule.cpp\
    MyModules/KeyReader.cpp\
    MyModules/MyModules.cpp\
    MyModules/VideoSource.cpp\
    MyModules/ImageProcessor.cpp

MYMODULES_OBJ=$(MYMODULES_SRC:.cpp=.obj)

all: cmodules mymodules slag

doc: doxy\config
	doxy\doxygen doxy\config

{Slag}.cpp{Slag}.obj::
	cl $(CPP_FLAGS) /I"$(ASYNCQUEUE_DIR)\inc" /Fo"Slag/" $<

{Slag\WIN}.cpp{Slag\WIN}.obj::
	cl $(CPP_FLAGS) /I"$(ASYNCQUEUE_DIR)\inc" /Fo"Slag/WIN/" $<

$(OUT_DIR)\SLAG.exe: $(SLAG_OBJ) "$(ASYNCQUEUE_LIB)"
	link $(WINAPI_LIB) "$(ASYNCQUEUE_LIB)" $(SLAG_OBJ) /OUT:$(OUT_DIR)\SLAG.exe

slag: $(OUT_DIR)\SLAG.exe

CModules/CModules.obj: CModules/CModules.c
	cl /c $(C_FLAGS) CModules/CModules.c /Fo"CModules/CModules.obj"

$(OUT_DIR)\CModules.dll: CModules/CModules.obj
	link $** /DLL /OUT:$(OUT_DIR)\CModules.dll /DEF:CModules\def.def

cmodules: $(OUT_DIR)\CModules.dll

OPENCV_LIBS=opencv_core$(OPENCV_VERSION).lib\
    opencv_highgui$(OPENCV_VERSION).lib\
    opencv_imgproc$(OPENCV_VERSION).lib

OPENCV_DLLS=$(OPENCV_LIBS:.lib=.dll)

{MyModules}.cpp{MyModules}.obj::
	cl $(CPP_FLAGS) /Fo"MyModules/" /I"$(OPENCV_DIR)\include" $<

$(OUT_DIR)\MyModules.dll: $(MYMODULES_OBJ)
	link /LIBPATH:"$(OPENCV_BIN)\lib" $(OPENCV_LIBS) $** /DLL /OUT:$(OUT_DIR)\MyModules.dll /DEF:MyModules\def.def
    cd "$(OPENCV_BIN)\bin"
    (
    robocopy "$(OPENCV_BIN)\bin" "$(MAKEDIR)\$(OUT_DIR)" $(OPENCV_DLLS) > nul && (echo Success) || (echo )
    )
    cd "$(MAKEDIR)"

mymodules: $(OUT_DIR)\MyModules.dll

clean:
	del /Q $(OUT_DIR)\*.* Slag\*.obj Slag\WIN\*.obj CModules\*.obj MyModules\*.obj
