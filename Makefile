include makeinclude

OBJ	= 	main.o \
		slogthread.o \
		logqueue.o \
		trafcount.o \
		tlogrotate.o

all: logsplit.exe
	mkapp logsplit

logsplit.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o logsplit.exe $(OBJ) $(LIBS)

clean:
	rm -f *.o *.exe
	rm -rf logsplit.app
	rm -f logsplit

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -g $<
