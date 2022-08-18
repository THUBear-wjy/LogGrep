PROG = lzma
CXX = gcc
TARGET   = liblzma.a
LIB =
RM = rm -f
CFLAGS = -c -O2 -Wall -D_7ZIP_ST -fPIC

OBJS = Alloc.o LzFind.o LzmaDec.o LzmaEnc.o 7zFile.o 7zStream.o LzmaLib.o
SRCS = Alloc.c LzFind.c LzmaDec.c LzmaEnc.c 7zFile.c 7zStream.c LzmaLib.c

all: $(PROG)

$(PROG): $(OBJS)
	ar cr $(TARGET) $(OBJS) 
	rm -f *.o

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $^
	
clean:
	-$(RM) $(PROG) $(OBJS)
	-$(RM) $(PROG) $(TARGET)
