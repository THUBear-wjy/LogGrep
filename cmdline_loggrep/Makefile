####### Output directory
FILE_DIR = 
TEMP_DIR = 
REFER_DIR = ../compression/
REFER2_DIR = ../compression/LZMA/
EXE_DIR = ../output/
#######################
RM	= rm -fr
DEL_FILE	= rm -f

CXX	= g++
#CPPFLAGS = -O0 -w -c -g -Wall -ggdb
CPPFLAGS = -O2 -w -c -g
LANGUAGE = -D	ENGLISH
READLINE = -D	READLINE_LIBRARY
##LANGUAGE = -D	CHINESE

BIN_OBJECT	= thulr_cmdline
LZMALIB_OBJECT = liblzma.a

AR	= ar -rc

####### Files

SRC_OBJECTS	=	$(FILE_DIR)BaseCmd.cpp \
	$(FILE_DIR)CmdDefine.cpp \
	$(FILE_DIR)CmdInvoker.cpp \
	$(FILE_DIR)CmdManager.cpp \
	$(FILE_DIR)CmdOperator.cpp \
	$(FILE_DIR)ConcreteCmd.cpp \
	$(FILE_DIR)CmdLineTool.cpp \
	$(FILE_DIR)LogStructure.cpp\
	$(FILE_DIR)SearchAlgorithm.cpp\
	$(FILE_DIR)LogStore_API.cpp\
	$(FILE_DIR)LogDispatcher.cpp\
	$(REFER_DIR)Coffer.cpp\

OBJECTS=	$(TEMP_DIR)BaseCmd.o \
	$(TEMP_DIR)CmdDefine.o \
	$(TEMP_DIR)CmdInvoker.o \
	$(TEMP_DIR)CmdManager.o \
	$(TEMP_DIR)CmdOperator.o \
	$(TEMP_DIR)ConcreteCmd.o \
	$(TEMP_DIR)CmdLineTool.o \
	$(TEMP_DIR)LogStructure.o\
	$(TEMP_DIR)SearchAlgorithm.o \
	$(TEMP_DIR)LogStore_API.o\
	$(TEMP_DIR)LogDispatcher.o\
	$(TEMP_DIR)Coffer.o\
		
H_OBJECTS	=	$(FILE_DIR)BaseCmd.h \
	$(FILE_DIR)CmdDefine.h \
	$(FILE_DIR)CmdInvoker.h \
	$(FILE_DIR)CmdManager.h \
	$(FILE_DIR)CmdOperator.h \
	$(FILE_DIR)ConcreteCmd.h \
	$(FILE_DIR)LogStructure.h\
	$(FILE_DIR)SearchAlgorithm.h\
	$(FILE_DIR)LogStore_API.h\
	$(FILE_DIR)LogDispatcher.h\
	$(REFER_DIR)Coffer.h\


####### Compile
#######-l dl: dlopen dlerror dlclose dlsym
######-l pthread: pthread_create
######-l rt:realtime

all: APP clean

APP: $(BIN_OBJECT)
	
$(BIN_OBJECT):$(OBJECTS)
	$(CXX) -o  $(BIN_OBJECT) $(OBJECTS)  $(REFER2_DIR)$(LZMALIB_OBJECT) -l dl
	cp $(BIN_OBJECT) $(EXE_DIR)
	
$(OBJECTS):$(SRC_OBJECTS) $(H_OBJECTS)
	$(CXX) $(CPPFLAGS) $(SRC_OBJECTS) $(LANGUAGE) $(READLINE)

.PHONY:clean

clean:
	$(RM) $(OBJECTS)
	



