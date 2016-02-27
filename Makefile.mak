POCODIR=E:\PROGRAMOK\POCO

POCO_INC=/I"$(POCODIR)\Foundation\include" /I"$(POCODIR)\Util\include"
# Win32
POCO_LIBPATH=$(POCODIR)\lib
OUT_DIR=bin

CL_FLAGS=/MT /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS /Og $(POCO_INC) /link /LIBPATH:"$(POCO_LIBPATH)"

all: slag cmodules mymodules doc

doc: doxy\config
	doxy\doxygen doxy\config

slag:

cmodules: CModules\CModules.c
	cl CModules\CModules.c /I"inc" /Fo:$(OUT_DIR)\CModules.obj /TC /MT /W3 /Og /link /DLL /OUT:$(OUT_DIR)\CModules.dll /DEF:CModules\def.def
mymodules:

test: AsyncQueueTest\AsyncQueueTest.cpp
	cl AsyncQueueTest\AsyncQueueTest.cpp /Fo:$(OUT_DIR)\AsyncQueueTest.obj $(CL_FLAGS) /OUT:$(OUT_DIR)\AsyncQueueTest.exe
clean:
	