PROJDIR = $(HOME)/projects/alphora/Fastore/trunk


CFLAGS += -g -I.. -I../../FastoreClient -I../../FastoreCommunication \
	-I$(PROJDIR)/External/Thrift/src \
	-I/usr/pkg/include

SRC = 	Wal.cpp \
	sqlite3/random.c \
	sqlite3/support.c

OBJ = 	$(SRC:R:S/$/.o/)

all: $(OBJ)