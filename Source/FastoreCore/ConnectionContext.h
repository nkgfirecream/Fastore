#include <thrift/transport/TTransport.h>
#include <thrift/protocol/TProtocol.h>

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

class TMultiConnectionContext
{
	public:

	boost::shared_ptr<TTransport> client;
	boost::shared_ptr<TTransport> inputTransport;
	boost::shared_ptr<TTransport> outputTransport;
	boost::shared_ptr<TProtocol> inputProtocol;
	boost::shared_ptr<TProtocol> outputProtocol;

};
