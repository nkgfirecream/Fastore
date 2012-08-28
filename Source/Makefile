# $Id$
DBG = -O0
CFLAGS += 
CXXFLAGS += -g -o $@ -fPIC \
	-std=c++0x \
	-I ./SQLite \
	-I /usr/local/include \
	-I /usr/pkg/include \
	-I /usr/local/include/thrift 

THRIFT.BASE =	Server_types.h \
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

THRIFT.SRC = $(THRIFT.BASE:S,^,FastoreCommunication/,g)
THRIFT.OBJ = $(THRIFT.SRC:M*.cpp:N*.skeleton.*:S/.cpp/.o/)

CORE.BASE != ls FastoreCore
CORE.SRC = $(CORE.BASE:M*.cpp:S,^,FastoreCore/,g)
CORE.OBJ = $(CORE.SRC:.cpp=.o)

MODULE.BASE != ls FastoreModule
MODULE.SRC = $(MODULE.BASE:M*.cpp:S,^,FastoreModule/,g)
MODULE.OBJ = $(MODULE.SRC:.cpp=.o)

PROVIDER.BASE != ls FastoreProvider
PROVIDER.SRC = $(PROVIDER.BASE:M*.cpp:S,^,FastoreProvider/,g)
PROVIDER.OBJ = $(PROVIDER.SRC:.cpp=.o)

SRC = $(CLIENT.SRC) $(THRIFT.SRC) $(CORE.SRC)

.PHONY: client 	FastoreClient \
	thrift	FastoreCommunication \
	core 	FastoreCore \
	module	FastoreModule \
	provide	FastoreProvider \
	FastoreService \
	FastoreShell \
	SQLite \
	packages

OBJ = $(THRIFT.OBJ) $(CORE.OBJ) $(MODULE.OBJ) $(PROVIDER.OBJ)
OUTPUT =   \
	FastoreCommunication/libthrift.so \
	FastoreCore/libfastore-core.so \
	FastoreModule/libfastore-module.so \
	FastoreProvider/libfastore-provider.so

all: $(THRIFT.SRC) $(OUTPUT)

client FastoreClient: FastoreClient/libfastore-client.so
FastoreClient/libfastore-client.so: $(CLIENT.OBJ)
	$(CXX) -shared $(.ALLSRC)
client-clean:
	rm -f $(CLIENT.OBJ)

thrift FastoreCommunication: FastoreCommunication/libthrift.so
FastoreCommunication/libthrift.so: $(THRIFT.OBJ)
	$(CXX) -shared $(.ALLSRC)
thrift-src: $(THRIFT.SRC)
thrift-clean: 
	rm -f $(THRIFT.SRC) $(THRIFT.OBJ) FastoreCommunication/libthrift.so

core FastoreCore: FastoreCore/libfastore-core.so
FastoreCore/libfastore-core.so: $(CORE.OBJ)
	$(CXX) -shared $(.ALLSRC)
core-clean:
	rm -f $(CORE.OBJ)

module FastoreModule: FastoreModule/libfastore-module.so
FastoreModule/libfastore-module.so: $(MODULE.OBJ)
	$(CXX) -shared $(.ALLSRC)
module-clean:
	rm -f $(MODULE.OBJ)

provide provider FastoreProvider: FastoreProvider/libfastore-provider.so
FastoreProvider/libfastore-provider.so: $(PROVIDER.OBJ)
	$(CXX) -shared $(.ALLSRC)
provider-clean:
	rm -f $(PROVIDER.OBJ)

clean: 
	rm -f $(THRIFT.OBJ) FastoreCommunication/libthrift.so


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
FastoreCore/Endpoint.o: /usr/include/sys/cdefs.h /usr/include/machine/cdefs.h
FastoreCore/Endpoint.o: /usr/include/sys/cdefs_elf.h /usr/include/sys/errno.h
FastoreCore/Endpoint.o: /usr/include/sys/featuretest.h /usr/include/poll.h
FastoreCore/Endpoint.o: /usr/include/sys/sigtypes.h
FastoreCore/Endpoint.o: /usr/include/machine/int_types.h
FastoreCore/Endpoint.o: /usr/include/machine/ansi.h
FastoreCore/Endpoint.o: /usr/include/sys/satypes.h
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
FastoreCore/ServiceHandler.o: /usr/include/errno.h /usr/include/sys/cdefs.h
FastoreCore/ServiceHandler.o: /usr/include/machine/cdefs.h
FastoreCore/ServiceHandler.o: /usr/include/sys/cdefs_elf.h
FastoreCore/ServiceHandler.o: /usr/include/sys/errno.h
FastoreCore/ServiceHandler.o: /usr/include/sys/featuretest.h
FastoreCore/ServiceHandler.o: /usr/include/poll.h /usr/include/sys/sigtypes.h
FastoreCore/ServiceHandler.o: /usr/include/machine/int_types.h
FastoreCore/ServiceHandler.o: /usr/include/machine/ansi.h
FastoreCore/ServiceHandler.o: /usr/include/sys/satypes.h
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
FastoreCore/TFastoreServer.o: /usr/include/errno.h /usr/include/sys/cdefs.h
FastoreCore/TFastoreServer.o: /usr/include/machine/cdefs.h
FastoreCore/TFastoreServer.o: /usr/include/sys/cdefs_elf.h
FastoreCore/TFastoreServer.o: /usr/include/sys/errno.h
FastoreCore/TFastoreServer.o: /usr/include/sys/featuretest.h
FastoreCore/TFastoreServer.o: /usr/include/poll.h /usr/include/sys/sigtypes.h
FastoreCore/TFastoreServer.o: /usr/include/machine/int_types.h
FastoreCore/TFastoreServer.o: /usr/include/machine/ansi.h
FastoreCore/TFastoreServer.o: /usr/include/sys/satypes.h
FastoreCore/TFastoreServer.o: /usr/include/assert.h
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
FastoreCore/WorkerHandler.o: /usr/include/errno.h /usr/include/sys/cdefs.h
FastoreCore/WorkerHandler.o: /usr/include/machine/cdefs.h
FastoreCore/WorkerHandler.o: /usr/include/sys/cdefs_elf.h
FastoreCore/WorkerHandler.o: /usr/include/sys/errno.h
FastoreCore/WorkerHandler.o: /usr/include/sys/featuretest.h
FastoreCore/WorkerHandler.o: /usr/include/poll.h /usr/include/sys/sigtypes.h
FastoreCore/WorkerHandler.o: /usr/include/machine/int_types.h
FastoreCore/WorkerHandler.o: /usr/include/machine/ansi.h
FastoreCore/WorkerHandler.o: /usr/include/sys/satypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/standardtypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/BaseTypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/StringTypes.h
FastoreCore/WorkerHandler.o: FastoreCore/Schema/InternalTypes.h
