include makeinclude

OBJ	= 	main.o \
		slogthread.o \
		logqueue.o \
		trafcount.o \
		tlogrotate.o

all: logax

logax: $(OBJ)
	$(LD) $(LDFLAGS) -o logax $(OBJ) $(LIBS)

clean:
	rm -f *.o
	rm -f logax

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -g $<
