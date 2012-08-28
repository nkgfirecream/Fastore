# $Id$
# Build thrift files
all: ../Server.thrift ../Comm.thrift
	thrift --gen cpp -strict -out . ../Comm.thrift
	thrift --gen cpp -strict -out . ../Server.thrift

