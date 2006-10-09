include makeinclude

OBJ	= 	main.o \
		slogthread.o \
		logqueue.o \
		trafcount.o \
		tlogrotate.o

all: logax.exe
	mkapp logax

logax.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o logax.exe $(OBJ) $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf logax.app
	rm -f logax

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -g $<
