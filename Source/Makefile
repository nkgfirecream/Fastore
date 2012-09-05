# $Id$
DBG = -O0
CFLAGS += 
#PERMISSIVE=-fpermissive
CXXFLAGS += -g -o $@ -fPIC $(PERMISSIVE) \
	-std=c++0x \
	-I SQLite \
	-I /usr/local/include \
	-I /usr/pkg/include \
	-I /usr/local/include/thrift 

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
COMM.SRC = $(COMM.BASE:S,^,FastoreCommunication/,g)
COMM.OBJ = $(COMM.SRC:M*.cpp:N*.skeleton.*:S/.cpp/.o/)

CORE.SRC != find FastoreCore -name '*.cpp' |  grep -v /Wal/
CORE.OBJ = $(CORE.SRC:.cpp=.o)

MODULE.BASE != ls FastoreModule
MODULE.SRC = $(MODULE.BASE:M*.cpp:S,^,FastoreModule/,g)
MODULE.OBJ = $(MODULE.SRC:.cpp=.o)

PROVIDER.BASE != ls FastoreProvider
PROVIDER.SRC = $(PROVIDER.BASE:M*.cpp:S,^,FastoreProvider/,g)
PROVIDER.OBJ = $(PROVIDER.SRC:.cpp=.o)

SERVICE.BASE != ls FastoreService/*.h FastoreService/fastored.cpp
SERVICE.SRC = $(SERVICE.BASE:M*.cpp)
SERVICE.OBJ = $(SERVICE.SRC:.cpp=.o)

SHELL.BASE != ls FastoreShell/*.h FastoreShell/*.cpp
SHELL.SRC = $(SHELL.BASE:M*.cpp)
SHELL.OBJ = $(SHELL.SRC:.cpp=.o)

SRC = $(CLIENT.SRC) $(COMM.SRC) $(CORE.SRC)

.PHONY: client 	FastoreClient \
	comm	FastoreCommunication \
	core 	FastoreCore \
	module	FastoreModule \
	provide	FastoreProvider \
	service	FastoreService \
	shell	FastoreShell \
	SQLite \
	packages

OBJ = $(COMM.OBJ) $(CORE.OBJ) $(MODULE.OBJ) $(PROVIDER.OBJ)
LIBS =   \
	FastoreClient/libfastore-client.so \
	FastoreCommunication/libfastore-communication.so \
	FastoreCore/libfastore-core.so \
	FastoreModule/libfastore-module.so \
	FastoreProvider/libfastore-provider.so

all: $(COMM.SRC) $(LIBS)

$(COMM.SRC): FastoreCommunication/.Comm.done FastoreCommunication/.Server.done

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
FastoreCommunication/libfastore-communication.so: $(COMM.OBJ)
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC)
thrift-src: $(COMM.SRC)
thrift-clean: 
	rm -f $(COMM.SRC) $(COMM.OBJ) \
		FastoreCommunication/libfastore-communication.so

core FastoreCore: FastoreCore/libfastore-core.so
FastoreCore/libfastore-core.so: $(CORE.OBJ)
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC)
core-clean:
	rm -f $(CORE.OBJ) FastoreCore/libfastore-core.so

MODULE.CLIENT.BASE = Client ClientException DataSet Dictionary Generator IDGenerator ServiceAddress Transaction
MODULE.CLIENT.OBJ = $(MODULE.CLIENT.BASE:S,^,FastoreClient/,g:S/$/.o/g)
module FastoreModule: FastoreModule/libfastore-module.so
FastoreModule/libfastore-module.so: $(MODULE.OBJ) $(MODULE.CLIENT.OBJ)
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC) 
module-clean:
	rm -f $(MODULE.OBJ) FastoreModule/libfastore-module.so

provide provider FastoreProvider: FastoreProvider/libfastore-provider.so
FastoreProvider/libfastore-provider.so: $(PROVIDER.OBJ) 
	$(CXX) $(CXXFLAGS) -shared $(.ALLSRC)
provider-clean:
	rm -f $(PROVIDER.OBJ) FastoreProvider/libfastore-provider.so

service FastoreService: FastoreService/libfastore-service.so
FastoreService/libfastore-service.so: $(SERVICE.OBJ) $(LIBS)
	$(CXX) $(CXXFLAGS) $(SERVICE.OBJ) \
		-L /usr/local/lib \
		$(LIBS:H:S,^,-L./,g) \
		$(LIBS:T:R:S/^lib/-l/g) \
		-lthrift -lsqlite3
service-clean:
	rm -f $(SERVICE.OBJ) FastoreService/libfastore-service.so

shell FastoreShell: FastoreShell/libfastore-shell.so
FastoreShell/libfastore-shell.so: $(SHELL.OBJ) $(LIBS)
	$(CXX) $(CXXFLAGS) $(SHELL.OBJ) \
		-L /usr/local/lib \
		$(LIBS:H:S,^,-L./,g) \
		$(LIBS:T:R:S/^lib/-l/g) \
		-lthrift -lsqlite3
shell-clean:
	rm -f $(SHELL.OBJ) FastoreShell/libfastore-shell.so

clean: 
	rm -f 	$(CLIENT.OBJ) \
		$(COMM.OBJ) \
		$(CORE.OBJ) \
		$(MODULE.OBJ) \
		$(PROVIDER.OBJ) \
		$(SERVICE.OBJ) \
		$(LIBS)

depend:
	makedepend -- $(CFLAGS) -- $(SRC)

##	$ ls -F  | grep /$ | grep -v [.]
##	FastoreClient/
##	FastoreCommunication/
##	FastoreCore/
##	FastoreModule/
##	FastoreProvider/
##	FastoreService/
##	FastoreShell/
##	SQLite/
##	packages/
# DO NOT DELETE

FastoreCommunication/Server_types.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Service.o: FastoreCommunication/Service.h
FastoreCommunication/Service.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Server_constants.o: FastoreCommunication/Server_constants.h
FastoreCommunication/Server_constants.o: FastoreCommunication/Server_types.h
FastoreCommunication/Server_constants.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Service.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Server_constants.o: FastoreCommunication/Server_types.h
FastoreCommunication/Server_constants.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Worker.o: FastoreCommunication/Worker.h
FastoreCommunication/Worker.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Service_server.skeleton.o: FastoreCommunication/Service.h
FastoreCommunication/Service_server.skeleton.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Worker.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Comm_types.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Worker_server.skeleton.o: FastoreCommunication/Worker.h
FastoreCommunication/Worker_server.skeleton.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Comm_constants.o: FastoreCommunication/Comm_constants.h
FastoreCommunication/Comm_constants.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Server_types.o: FastoreCommunication/Server_types.h
FastoreCommunication/Server_types.o: FastoreCommunication/Comm_types.h
FastoreCommunication/Comm_constants.o: FastoreCommunication/Comm_types.h
FastoreCore/BTree.o: FastoreCore/BTree.h FastoreCore/Schema/scalar.h
FastoreCore/BTree.o: FastoreCore/Schema/standardtypes.h
FastoreCore/BTree.o: FastoreCore/Schema/BaseTypes.h
FastoreCore/BTree.o: FastoreCore/Schema/scalar.h
FastoreCore/BTree.o: FastoreCore/Schema/StringTypes.h
FastoreCore/BTree.o: FastoreCore/Schema/InternalTypes.h
FastoreCore/BTree.o: FastoreCore/treeentry.h
FastoreCore/Endpoint.o: FastoreCore/Endpoint.h FastoreCore/EndpointConfig.h
FastoreCore/Endpoint.o: FastoreCore/TFastoreServer.h /usr/include/errno.h
FastoreCore/Repository.o: FastoreCore/Repository.h
FastoreCore/Repository.o: FastoreCore/Schema/column.h
FastoreCore/Repository.o: FastoreCore/Schema/scalar.h
FastoreCore/Repository.o: FastoreCore/Column/IColumnBuffer.h
FastoreCore/Repository.o: FastoreCore/Schema/scalar.h
FastoreCore/Repository.o: FastoreCommunication/Comm_types.h
FastoreCore/Repository.o: FastoreCommunication/Comm_types.h
FastoreCore/Repository.o: FastoreCore/Column/UniqueBuffer.h
FastoreCore/Repository.o: FastoreCore/Schema/standardtypes.h
FastoreCore/Repository.o: FastoreCore/Schema/BaseTypes.h
FastoreCore/Repository.o: FastoreCore/Schema/StringTypes.h
FastoreCore/Repository.o: FastoreCore/Schema/InternalTypes.h
FastoreCore/Repository.o: FastoreCore/BTree.h FastoreCore/Schema/scalar.h
FastoreCore/Repository.o: FastoreCore/Schema/standardtypes.h
FastoreCore/Repository.o: FastoreCore/treeentry.h
FastoreCore/Repository.o: FastoreCore/Column/IColumnBuffer.h
FastoreCore/Repository.o: FastoreCore/Column/TreeBuffer.h
FastoreCore/Repository.o: FastoreCore/Column/IdentityBuffer.h
FastoreCore/Scheduler.o: FastoreCore/Scheduler.h
FastoreCore/Scheduler.o: FastoreCommunication/Comm_types.h
FastoreCore/Serialization.o: FastoreCore/Serialization.h
FastoreCore/Serialization.o: FastoreCore/Column/IColumnBuffer.h
FastoreCore/Serialization.o: FastoreCore/Schema/scalar.h
FastoreCore/Serialization.o: FastoreCommunication/Comm_types.h
FastoreCore/Serialization.o: FastoreCore/Schema/scalar.h
FastoreCore/Serialization.o: FastoreCore/TFastoreFileTransport.h
FastoreCore/ServiceHandler.o: FastoreCore/ServiceHandler.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Service.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Server_types.h
FastoreCore/ServiceHandler.o: FastoreCore/Endpoint.h
FastoreCore/ServiceHandler.o: FastoreCore/EndpointConfig.h
FastoreCore/ServiceHandler.o: FastoreCore/Scheduler.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/ServiceHandler.o: FastoreCore/TFastoreServer.h
FastoreCore/ServiceHandler.o: FastoreCore/WorkerHandler.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Worker.h
FastoreCore/ServiceHandler.o: FastoreCore/Repository.h
FastoreCore/ServiceHandler.o: FastoreCore/Schema/column.h
FastoreCore/ServiceHandler.o: FastoreCore/Schema/scalar.h
FastoreCore/ServiceHandler.o: FastoreCore/Column/IColumnBuffer.h
FastoreCore/ServiceHandler.o: FastoreCore/Schema/scalar.h
FastoreCore/ServiceHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/TFastoreFileTransport.o: FastoreCore/TFastoreFileTransport.h
FastoreCore/TFastoreServer.o: FastoreCore/TFastoreServer.h
FastoreCore/WorkerHandler.o: FastoreCore/WorkerHandler.h
FastoreCore/WorkerHandler.o: FastoreCommunication/Worker.h
FastoreCore/WorkerHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/WorkerHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/WorkerHandler.o: FastoreCore/Repository.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/column.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/scalar.h
FastoreCore/WorkerHandler.o: FastoreCore/Column/IColumnBuffer.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/scalar.h
FastoreCore/WorkerHandler.o: FastoreCommunication/Comm_types.h
FastoreCore/WorkerHandler.o: FastoreCore/Scheduler.h
FastoreCore/WorkerHandler.o: FastoreCore/TFastoreServer.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/standardtypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/BaseTypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/StringTypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/InternalTypes.h
