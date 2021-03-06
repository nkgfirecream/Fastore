# $Id$
.SUFFIXES: .i .dep

# This HAVE business is made necessary because thrift's config.h 
# fails to mention many installed include files.  
HAVE.LIST = poll.h netdb.h fcntl.h
HAVE.FILES != find /usr/include -maxdepth 1 \
		\( $(HAVE.LIST:S/^/-or -name /g:S/^-or//1) \) \
		&& test -f /usr/include/netinet/in.h && echo netinet_in.h
HAVE.SYMS = $(HAVE.FILES:T:tu:S/./_/g:S/^/HAVE_/g) USE_WAL DEBUG_WAL_FUNC
HAVES = $(HAVE.SYMS:S/^/-D/g)

DBG = -O0
#CFLAGS += 
#PERMISSIVE=-fpermissive
PERMISSIVE = -Wall -Werror -Wno-reorder -Wconversion
CXXDIRS = -I /usr/local/include
PREFIX = /usr/local
THRIFT.LIB = $(PREFIX)/lib/libthrift.so

CXXFLAGS += -g -o $@ -fPIC $(PERMISSIVE) -std=c++0x $(HAVES) $(CXXDIRS)

COMM.THRIFT =	Server_types.h \
		Service.cpp \
		Server_constants.cpp \
		Service.h \
		Server_constants.h \
		Worker.cpp \
		Service_server.skeleton.cpp \
		Worker.h \
		Comm_types.cpp \
		Worker_server.skeleton.cpp \
		Comm_types.h \
		Comm_constants.cpp \
		Server_types.cpp \
		Comm_constants.h

CLIENT.BASE != ls FastoreClient
CLIENT.SRC = $(CLIENT.BASE:M*.cpp:S,^,FastoreClient/,g)
CLIENT.OBJ = $(CLIENT.SRC:.cpp=.o)

COMM.BASE = $(COMM.THRIFT) Operators.cpp
COMM.FILES = $(COMM.BASE:S,^,FastoreCommunication/,g)
COMM.SRC = $(COMM.FILES:M*.cpp:N*.skeleton.*)
COMM.OBJ = $(COMM.SRC:S/.cpp/.o/)

CORE.SRC != find FastoreCore -name '*.cpp' |  grep -Ev '/t/|/OK/|Faux'
CORE.OBJ = $(CORE.SRC:N*/wald.cpp:.cpp=.o)

MODULE.BASE != ls FastoreModule
MODULE.SRC = $(MODULE.BASE:M*.cpp:S,^,FastoreModule/,g)
MODULE.OBJ = $(MODULE.SRC:.cpp=.o)

PROVIDER.BASE != ls FastoreProvider
PROVIDER.SRC = $(PROVIDER.BASE:M*.cpp:S,^,FastoreProvider/,g)
PROVIDER.OBJ = $(PROVIDER.SRC:.cpp=.o)

SERVICE.BASE != ls FastoreService/*.h FastoreService/fastored.cpp
SERVICE.SRC = $(SERVICE.BASE:M*.cpp)
SERVICE.OBJ = $(SERVICE.SRC:.cpp=.o)

SHELL.BASE != ls FastoreShell/*.h FastoreShell/*.c
SHELL.SRC = $(SHELL.BASE:M*.c)
SHELL.OBJ = $(SHELL.SRC:.c=.o)

WALD.SRC = FastoreCore/Wal/wald.cpp
WALD.OBJ = $(WALD.SRC:.cpp=.o)

SRC = 	$(CLIENT.SRC) \
	$(COMM.SRC) \
	$(CORE.SRC) \
	$(MODULE.SRC) \
	$(PROVIDER.SRC) \
	$(SERVICE.SRC) \
	$(SHELL.SRC)
DEPENDS = $(SRC:R:S/$/.dep/g)

.PHONY: client 	FastoreClient \
	comm	FastoreCommunication \
	core 	FastoreCore \
	module	FastoreModule \
	provide	FastoreProvider \
	service	FastoreService \
	shell	FastoreShell \
	sqlite	SQLite 

OBJ = $(COMM.OBJ) $(CORE.OBJ) $(MODULE.OBJ) $(PROVIDER.OBJ)
LIBS =  FastoreClient/libfastore-client.so \
	FastoreCommunication/libfastore-communication.so \
	FastoreCore/libfastore-core.so \
	FastoreModule/libfastore-module.so \
	FastoreProvider/libfastore-provider.so 
BINS = 	FastoreService/fastored \
	FastoreShell/shell \
	FastoreCore/Wal/wald
ELFS = $(LIBS) $(BINS)
INSTALLED = 	$(LIBS:T:S,^,$(PREFIX)/lib/,g) \
		$(BINS:T:S,^,$(PREFIX)/bin/,g)

all: $(COMM.THRIFT) $(LIBS) $(BINS)

install: $(INSTALLED)
$(INSTALLED): 
	install $(ELFS:M*/$(@:T)) $@ 

$(PREFIX)/lib/libfastore-client.so: FastoreClient/libfastore-client.so
$(PREFIX)/lib/libfastore-communication.so: FastoreCommunication/libfastore-communication.so
$(PREFIX)/lib/libfastore-core.so: FastoreCore/libfastore-core.so
$(PREFIX)/lib/libfastore-module.so: FastoreModule/libfastore-module.so
$(PREFIX)/lib/libfastore-provider.so: FastoreProvider/libfastore-provider.so 
$(PREFIX)/bin/fastored: FastoreService/fastored
$(PREFIX)/bin/shell: FastoreShell/shell

$(COMM.THRIFT): FastoreCommunication/.Comm.done FastoreCommunication/.Server.done

FastoreCommunication/.Comm.done: Comm.thrift
	thrift --gen cpp -strict -out FastoreCommunication/ Comm.thrift
	@touch $@
FastoreCommunication/.Server.done: Server.thrift
	thrift --gen cpp -strict -out FastoreCommunication/ Server.thrift
	@touch $@

client FastoreClient: FastoreClient/libfastore-client.so
FastoreClient/libfastore-client.so: $(CLIENT.OBJ)
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC)
client-clean:
	rm -f $(CLIENT.OBJ) FastoreClient/libfastore-client.so

comm FastoreCommunication: FastoreCommunication/libfastore-communication.so
FastoreCommunication/libfastore-communication.so: $(COMM.OBJ) $(THRIFT.LIB)
	$(CXX) $(CXXFLAGS) -shared $(COMM.OBJ) \
		-Wl,-L$(PREFIX)/lib \
		-Wl,-rpath,$(PREFIX)/lib \
		-lthrift -lboost_thread -lboost_filesystem

thrift-src: $(COMM.THRIFT)
comm-clean thrift-clean: 
	rm -f $(COMM.THRIFT) $(COMM.OBJ) \
		FastoreCommunication/.Comm.done \
		FastoreCommunication/.Server.done \
		FastoreCommunication/libfastore-communication.so

core FastoreCore: FastoreCore/libfastore-core.so
FastoreCore/libfastore-core.so: $(CORE.OBJ) $(THRIFT.LIB)
	$(CXX) $(CXXFLAGS) -shared $(CORE.OBJ) \
		-LFastoreCommunication -L$(PREFIX)/lib \
		-Wl,-rpath,$(PREFIX)/lib \
		-lfastore-communication  \
		-lthrift -lboost_filesystem -lboost_thread 
core-clean:
	rm -f $(CORE.OBJ) FastoreCore/libfastore-core.so

# FastoreModule depends on some client files.
MODULE.CLIENT.BASE = 	Client \
			ClientException \
			Database \
			DataSet \
			Dictionary \
			Generator \
			IDGenerator \
			ServiceAddress \
			Transaction
MODULE.CLIENT.OBJ = $(MODULE.CLIENT.BASE:S,^,FastoreClient/,g:S/$/.o/g)

MODULE.COMMUNICATION.BASE = Service Worker
MODULE.COMMUNICATION.OBJ = $(MODULE.COMMUNICATION.BASE:S,^,FastoreCommunication/,g:S/$/.o/g)

module FastoreModule: FastoreModule/libfastore-module.so
FastoreModule/libfastore-module.so: 	$(MODULE.OBJ) \
					$(MODULE.CLIENT.OBJ) \
				$(PREFIX)/lib/libfastore-communication.so
	$(CXX) $(CXXFLAGS) -shared $(MODULE.OBJ) $(MODULE.CLIENT.OBJ) \
		-L FastoreCore -L$(PREFIX)/lib \
		-Wl,-rpath,$(PREFIX)/lib \
		-lfastore-core -lfastore-communication -lboost_thread
module-clean:
	rm -f $(MODULE.OBJ) FastoreModule/libfastore-module.so

provider FastoreProvider: FastoreProvider/libfastore-provider.so
FastoreProvider/libfastore-provider.so: $(PROVIDER.OBJ) 
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC)
provider-clean:
	rm -f $(PROVIDER.OBJ) FastoreProvider/libfastore-provider.so

BOOST_LIBS = filesystem system thread 

PIDFILE.C = FastoreCore/Wal/pidfile.c
fastored service FastoreService: FastoreService/fastored
FastoreService/fastored: $(SERVICE.OBJ) $(LIBS) $(PIDFILE.C) $(THRIFT.LIB)
	$(CXX) $(CXXFLAGS) $(SERVICE.OBJ) $(PIDFILE.C) \
		-L $(PREFIX)/lib \
		-Wl,-rpath -Wl,$(PREFIX)/lib \
		$(LIBS:H:S,^,-L./,g) \
		$(LIBS:T:R:S/^lib/-l/g) \
		-lthrift $(BOOST_LIBS:S/^/-lboost_/g) -lsqlite3
service-clean:
	rm -f $(SERVICE.OBJ) FastoreService/libfastore-service.so

SHELL.CXXFLAGS = $(CXXFLAGS:N-Wno-reorder:N-std=c++0x) -DHAVE_READLINE
shell FastoreShell: FastoreShell/shell
FastoreShell/shell: $(SHELL.SRC) $(THRIFT.LIB)
	$(CC) $(SHELL.CXXFLAGS) $(SHELL.SRC) \
		-L $(PREFIX)/lib \
		$(LIBS:H:S,^,-L./,g) \
		-Wl,-rpath -Wl,$(PREFIX)/lib \
		$(LIBS:T:R:S/^lib/-l/g) \
		-lthrift $(BOOST_LIBS:S/^/-lboost_/g) -lsqlite3 -lreadline
shell-clean:
	rm -f $(SHELL.OBJ) FastoreShell/shell

wald: FastoreCore/Wal/Wald
FastoreCore/Wal/wald: $(WALD.SRC) $(@:H)/pidfile.c
	$(CXX) $(CXXFLAGS:N-Wno-reorder:N-std=c++0x) $(.ALLSRC) \
		-Wl,-L$(@:H:H),-L$(PREFIX)/lib,-rpath,$(PREFIX)/lib \
		-lfastore-communication -lfastore-core
wald-clean:
	rm -f $(WALD.OBJ) FastoreCore/Wal/wald

clean: 
	rm -f 	$(CLIENT.OBJ) \
		$(COMM.OBJ) \
		$(CORE.OBJ) \
		$(MODULE.OBJ) \
		$(PROVIDER.OBJ) \
		$(SERVICE.OBJ) \
		$(SHELL.OBJ) \
		$(LIBS)

.cpp.i:
	$(CXX) -E $(CXXFLAGS) -c $(.ALLSRC)

cscope.files: $(SRC)
	echo $(CXXDIRS) | sed 's/-I  */-I/g' | tr \  \\n | grep '\-I' > $@~
	find . \( -name \*.c -or -name \*.cpp -or -name \*.h \) \
		| grep -v '\.skeleton\.' >> $@~
	mv $@~ $@

SQLite/build/Makefile:
	test -d $(@:H) || mkdir $(@:H)
	cd $(@:H) && ../configure

sqlite: SQLite/build/libsqlite3.so
SQLite/build/libsqlite3.so: SQLite/build/Makefile
	cd $(@:H) && make && make install

cscope.compare: cscope.files
	sed -ne '/^-I/p' cscope.files > $@~
	sed  '/^-I/d; s/[:\\]//g' .depend \
	| tr \  \\n  \
	| sed  '/\.o/d; /^[[:space:]]*$$/d' \
	| sort | uniq >> $@~
	mv $@~ $@

depend depends: .depend 
.depend: $(DEPENDS)
	cat $(DEPENDS) > $@~ 
	@mv $@~ $@
	rm $(DEPENDS)

$(DEPENDS):
	@$(CXX) -MM -MT$(@:R).o -MF$@ $(CXXFLAGS) \
		$(@:S/.dep/.cpp/g:S/shell.cpp/shell.c/)


