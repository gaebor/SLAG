POCODIR=E:\PROGRAMOK\POCO

POCO_INC=/I"$(POCODIR)\Foundation\include" /I"$(POCODIR)\Util\include"
# Win32
POCO_LIBPATH=$(POCODIR)\lib
OUT_DIR=bin

CL_FLAGS=/MT /W3 /EHsc /Ox $(POCO_INC) /link /LIBPATH:"$(POCO_LIBPATH)"

SOURCES=Slag/ConfigReader.cpp\
Slag/Factory.cpp\
Slag/HumanReadable.cpp\
Slag/InternalTypes.cpp\
Slag/ModuleIdentifier.cpp\
Slag/ModuleWrapper.cpp\
Slag/OutputText.cpp\
Slag/Slag.cpp\
Slag/Timer.cpp\
Slag/WIN/Imshow.obj\
Slag/WIN/LoadLibrary.obj\
Slag/WIN/TerminationSignal.obj

all: test cmodules mymodules main

doc: doxy\config
	doxy\doxygen doxy\config

INTERFACE=/I"inc"
WINAPI_LIB=shlwapi.lib

{Slag}.cpp{Slag}.obj::
	cl $< $(INTERFACE) $(CL_FLAGS) -Fd:build\

{Slag\WIN}.cpp{Slag\WIN}.obj::
	cl $< $(INTERFACE) $(CL_FLAGS)

main: $(SOURCES)
	link $(SOURCES) $(WINAPI_LIB) /OUT:$(OUT_DIR)\SLAG.exe

cmodules: CModules\CModules.c
	cl CModules\CModules.c $(INTERFACE) /Fo:$(OUT_DIR)\CModules.obj /TC /MT /W3 /Ox /link /DLL /OUT:$(OUT_DIR)\CModules.dll /DEF:CModules\def.def
mymodules:
	
test: AsyncQueueTest\AsyncQueueTest.cpp
	cl AsyncQueueTest\AsyncQueueTest.cpp /Fo:$(OUT_DIR)\AsyncQueueTest.obj $(CL_FLAGS) /OUT:$(OUT_DIR)\AsyncQueueTest.exe
clean:
	del bin\*.exe bin\*.obj bin\*.exp bin\*.lib bin\*.dll