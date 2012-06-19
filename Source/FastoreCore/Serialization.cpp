#include "Serialization.h"

//Buffer serializer
BufferSerializer::BufferSerializer(IColumnBuffer& buffer, string filename) : _buffer(buffer)
{
	_outputFile = filename;
	_firstWrite = true;
	_disposed = true;
}

bool BufferSerializer::open()
{
	if (!_disposed)
		throw "Serializer already open!";

	//acquire resources
	_transport = boost::shared_ptr<transport::TFileTransport>(new transport::TFileTransport(_outputFile, false));
	_protocol = boost::shared_ptr<protocol::TDenseProtocol>(new protocol::TDenseProtocol(_transport));

	_transport->open();

	_disposed = false;
}

bool BufferSerializer::close()
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

	if (result.valueRowsList.size() > 0)
	{
		writeResult(result);

		ValueRows lastValueRows = result.valueRowsList.at(result.valueRowsList.size() - 1);

		_lastValue = lastValueRows.value;
		_lastRowId = lastValueRows.rowIDs.at(lastValueRows.rowIDs.size() - 1);

		return result.endOfRange;
	}
	else
		return true;
}

void BufferSerializer::writeResult(fastore::communication::RangeResult& result)
{
	result.write(_protocol.get());
}

//Buffer deserializer
BufferDeserializer::BufferDeserializer(IColumnBuffer& buffer, std::string filename) : _buffer(buffer)
{
	_inputFile = filename;
	_disposed = true;
}

bool BufferDeserializer::open()
{
	if (!_disposed)
		throw "Serializer already open!";

	//acquire resources
	boost::shared_ptr<transport::TFileTransport> transport(new transport::TFileTransport(_inputFile, true));
	_protocol = boost::shared_ptr<protocol::TDenseProtocol>(new protocol::TDenseProtocol(transport));

	_disposed = false;
}

bool BufferDeserializer::close()
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
	//Test for more data
	if (!_transport->peek())
		return false;

	RangeResult result;
	result.read(_protocol.get());

	ColumnWrites writes;
	vector<Include> includes;

	ValueRowsList data = result.valueRowsList;
	for (int i = 0; i < data.size(); i++)
	{
		ValueRows vr = data.at(i);
		for (int j = 0; j < vr.rowIDs.size(); j++)
		{
			Include inc;
			inc.__set_value(vr.value);
			inc.__set_rowID(vr.rowIDs.at(j));
		}
	}

	writes.__set_includes(includes);
	_buffer.Apply(writes);
}