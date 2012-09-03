.SUFFIXES: .ms .dvi .ps .pdf

PROJDIR = $(HOME)/projects/alphora/Fastore/trunk

UNAME != uname
.if $(UNAME) == Darwin
#STD = 	-std=c++98
CXX = clang++
.else
STD =  	-std=c++0x
.endif

# 	-I../../../External/boost_1_49_0 
MACDIR = /Developer/SDKs/MacOSX10.6.sdk/usr
CFLAGS += -o $@ -g  $(STD) \
	-DDEBUG_WAL_FUNC \
	-I.. \
	-I../../../External/Thrift/src \
	-I../../FastoreClient \
	-I../../FastoreCommunication \
	-I/usr/local/include \
	-I/usr/pkg/include \
	-I $(MACDIR)/include

LDFLAGS = -lutil

SRC = 	Wal.cpp WalFile.cpp

SQLITE =	sqlite3/random.c \
		sqlite3/randomness.parts.c \
		sqlite3/support.c

OBJ = 	$(SRC:R:S/$/.o/)

WalFile.cpp: WalFile.h
wald.cpp: WalFile.h
wald: wald.cpp WalFile.o Wal.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(.ALLSRC)


all: $(OBJ)

clean: 
	rm -f $(OBJ) libwal.a t/wal_test t/wal_test.o

test: t/wal_test

libwal.a: $(OBJ)
	ar rc $@ $(.ALLSRC)

t/wal_test: t/wal_test.cpp libwal.a
	c++ $(CFLAGS) $(@:R).cpp -L. -lwal

.ms.ps:
	dformat $(.ALLSRC) | groff -pet -ms -Tps > $@~
	@ mv $@~ $@
.ps.pdf:
	pstopdf -o $@ $(.ALLSRC) 
