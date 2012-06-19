#include "Column\IColumnBuffer.h"
#include "Schema\scalar.h"
#include <string>
#include <fstream>

#include <boost\shared_ptr.hpp>
#include <thrift\protocol\TDenseProtocol.h>
#include <thrift\transport\TFileTransport.h>

using namespace std;
using namespace apache::thrift;
using namespace fastore::communication;

//Reading and writing through the "front door"
//allows the operation to happen in pieces so that
//We can continue to support reads

const int BufferChunkSize = 500;

class BufferSerializer
{
	public:
		BufferSerializer(IColumnBuffer& buffer, string filename);
		~BufferSerializer();

		//Open acquires resources
		bool open();

		//Returns true for EOF;
		bool writeNextChunk();

		//Close releases resources
		bool close();

	private:
		IColumnBuffer& _buffer;
		string _outputFile;

		bool _firstWrite;
		string _lastValue;
		string _lastRowId;

		boost::shared_ptr<protocol::TDenseProtocol> _protocol;
		boost::shared_ptr<transport::TFileTransport> _transport;

		bool _disposed;

		void writeResult(RangeResult& result);
};

class BufferDeserializer
{
	public:
		BufferDeserializer(IColumnBuffer& buffer, string filename);
		~BufferDeserializer();

		//Open acquires resources
		bool open();
		//Returns true for EOF;
		bool readNextChunk();
		//Close releases resources
		bool close();

	private:
		IColumnBuffer& _buffer;
		string _inputFile;

		boost::shared_ptr<protocol::TDenseProtocol> _protocol;
		boost::shared_ptr<transport::TFileTransport> _transport;

		bool _disposed;
};