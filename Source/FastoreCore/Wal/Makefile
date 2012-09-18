.SUFFIXES: .ms .dvi .ps .pdf .i

PROJDIR = $(HOME)/projects/alphora/Fastore/trunk

UNAME != uname
.if $(UNAME) == Darwin
#STD = 	-std=c++98
CXX = clang++
.else
STD =  	-std=c++0x
.endif

MACDIR = /Developer/SDKs/MacOSX10.6.sdk/usr
CXXFLAGS += -o $@ -g  $(STD) \
	-DDEBUG_WAL_FUNC \
	-I.. \
	-I../../FastoreClient \
	-I../../FastoreCommunication \
	-I/usr/local/include \
	-I $(MACDIR)/include

LDFLAGS = -lutil

SRC = 	Wal.cpp WalFile.cpp

SQLITE =	sqlite3/random.c \
		sqlite3/randomness.parts.c \
		sqlite3/support.c

OBJ = 	$(SRC:R:S/$/.o/)

wald: wald.cpp $(OBJ) pidfile.c
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(.ALLSRC) -lcrypto


WalFile.cpp: WalFile.h
wald.cpp: WalFile.h

clean: 
	rm -f $(OBJ) libwal.a t/wal_test t/wal_test.o

test: t/wal_test

libwal.a: $(OBJ)
	ar rc $@ $(.ALLSRC)

t/wal_test: t/wal_test.cpp libwal.a
	c++ $(CFLAGS) $(@:R).cpp -L. -lwal

.cpp.i:
	$(CXX) -E $(CXXFLAGS) -c $(.ALLSRC)

.ms.ps:
	dformat $(.ALLSRC) | groff -pet -ms -Tps > $@~
	@ mv $@~ $@
.ps.pdf:
	pstopdf -o $@ $(.ALLSRC) 
