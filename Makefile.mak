POCODIR=E:\PROGRAMOK\POCO

POCO_INC=/I"$(POCODIR)\Foundation\include" /I"$(POCODIR)\Util\include"
# Win32
POCO_LIBPATH=$(POCODIR)\lib
OUT_DIR=bin\Win32\Release

CL_FLAGS=/MT /W3 /D_CRT_SECURE_NO_WARNINGS /Ox $(POCO_INC) /link /LIBPATH:"$(POCO_LIBPATH)"

all: slag cmodules mymodules doc

doc: doxy\config
	doxy\doxygen doxy\config

slag:

cmodules:

mymodules:

test: AsyncQueueTest\AsyncQueueTest.cpp
	cl AsyncQueueTest\AsyncQueueTest.cpp /Fo:$(OUT_DIR)\AsyncQueueTest.obj $(CL_FLAGS) /OUT:$(OUT_DIR)\AsyncQueueTest.exe
clean:
	