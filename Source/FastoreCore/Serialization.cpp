#include "Serialization.h"
#include <thrift\protocol\TJSONProtocol.h>
#include "TFastoreFileTransport.h"


//Buffer serializer
BufferSerializer::BufferSerializer(IColumnBuffer& buffer, string filename) : _buffer(buffer)
{
	_outputFile = filename;
	_firstWrite = true;
	_disposed = true;
}

void BufferSerializer::open()
{
	if (!_disposed)
		throw "Serializer already open!";

	//acquire resources
	FILE* file = fopen(_outputFile.c_str(), "w");

	_transport = boost::shared_ptr<transport::TFastoreFileTransport>(new transport::TFastoreFileTransport(file));
	_protocol = boost::shared_ptr<protocol::TJSONProtocol>(new protocol::TJSONProtocol(_transport));

	_disposed = false;
}

void BufferSerializer::close()
{
	if (_disposed)
		throw "Serializer already closed!";

	//release resources
	_transport->flush();
	_transport->close();
	_transport.reset();
	_protocol.reset();

	_disposed = true;	
}

BufferSerializer::~BufferSerializer()
{
	if (!_disposed)
		close();
}

bool BufferSerializer::writeNextChunk()
{
	if(_disposed)
		throw "Serializer not opened!";

	fastore::communication::RangeRequest range;

	if (!_firstWrite)
	{
		fastore::communication::RangeBound bound;
		bound.__set_inclusive(true);
		bound.__set_value(_lastValue);

		range.__set_rowID(_lastRowId);
		range.__set_first(bound);
	}
	else
	{
		_firstWrite = false;
	}

	range.__set_ascending(true);
	range.__set_limit(BufferChunkSize);	
	RangeResult result = _buffer.GetRows(range);

	ValueRowsList vrl = result.valueRowsList;
	if (vrl.size() > 0)
	{
		writeValueRowsList(vrl);

		ValueRows lastValueRows = vrl.at(vrl.size() - 1);

		_lastValue = lastValueRows.value;
		_lastRowId = lastValueRows.rowIDs.at(lastValueRows.rowIDs.size() - 1);

		return !result.eof;
	}
	else
		return false;
}

void BufferSerializer::writeValueRowsList(fastore::communication::ValueRowsList& list)
{
	for (int i = 0; i < list.size(); i++)
	{
		list.at(i).write(_protocol.get());
	}
}

//Buffer deserializer
BufferDeserializer::BufferDeserializer(IColumnBuffer& buffer, std::string filename) : _buffer(buffer)
{
	_inputFile = filename;
	_disposed = true;
}

void BufferDeserializer::open()
{
	if (!_disposed)
		throw "Serializer already open!";

	//acquire resources
	FILE* file = fopen(_inputFile.c_str(), "r");

	_transport = boost::shared_ptr<transport::TFastoreFileTransport>(new transport::TFastoreFileTransport(file));
	_protocol = boost::shared_ptr<protocol::TJSONProtocol>(new protocol::TJSONProtocol(_transport));

	_disposed = false;
}

void BufferDeserializer::close()
{
	if (_disposed)
		throw "Serializer already closed!";

	//release resources
	_transport->close();
	_transport.reset();
	_protocol.reset();

	_disposed = true;	
}

BufferDeserializer::~BufferDeserializer()
{
	if (!_disposed)
		close();
}

bool BufferDeserializer::readNextChunk()
{
	if(_disposed)
		throw "Serializer not opened!";

	//Test for more data
	if (!_transport->peek())
		return false;

	int totalWritesMade = 0;
	ColumnWrites writes;
	vector<Include> includes(BufferChunkSize);

	//We can't stop mid structure... So we may go over the chunk size, but never more than 2x the chunksize
	while (totalWritesMade < BufferChunkSize && _transport->peek())
	{
		ValueRows vr;
		vr.read(_protocol.get());
		for (int j = 0; j < vr.rowIDs.size(); j++)
		{
			Include inc;
			inc.__set_value(vr.value);
			inc.__set_rowID(vr.rowIDs.at(j));
			includes.push_back(inc);
			totalWritesMade++;
		}
	}

	writes.__set_includes(includes);
	_buffer.Apply(writes);

	return true;
}