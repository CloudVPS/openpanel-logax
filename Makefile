include makeinclude

OBJ	= 	main.o \
		slogthread.o \
		logqueue.o \
		trafcount.o \
		tlogrotate.o

all: logaxe.exe
	mkapp logaxe

logaxe.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o logaxe.exe $(OBJ) $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf logaxe.app
	rm -f logaxe

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -g $<
