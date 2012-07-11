//Might not be needed.. Still exploring..

#include <thrift/transport/TSocket.h>
#include <functional>

using namespace apache::thrift::transport;

class TParkableSocket : public TSocket
{
	public:
	
	TParkableSocket(int socket); 
	TParkableSocket(std::string path);
	TParkableSocket(std::string host, int port);
	TParkableSocket();

	void resume();
	void setParkCallback(std::function<void(int)> callback);

	private:
	
	std::function<void(int)> _callback;
};
