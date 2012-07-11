#include "TParkableSocket.h"

TParkableSocket::TParkableSocket(int socket)
	: TSocket(socket) { }

TParkableSocket::TParkableSocket(std::string host, int port)
	: TSocket(host, port) { }

TParkableSocket::TParkableSocket(std::string path)
	: TSocket(path) { }

TParkableSocket::TParkableSocket()
	: TSocket() { }

void TParkableSocket::setParkCallback(std::function<void(int)> callback)
{
	_callback = callback;
}