#define __STDC_FORMAT_MACROS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _WIN32
#include <thrift/windows/force_inc.h>
#endif

#include "safe_cast.h"
#include "TFastoreServer.h"
#include <thrift/transport/TSocket.h>

#include <iostream>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <errno.h>
#include <assert.h>
#include <signal.h>

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifdef _MSC_VER
#define PRIu32 "I32u"
#endif

#include "Log/Syslog.h"

namespace apache { namespace thrift { namespace server {

using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace std;
using apache::thrift::transport::TSocket;
using apache::thrift::transport::TTransportException;
using boost::shared_ptr;

using fastore::Log;
using fastore::log_info;
using fastore::log_err;
using fastore::log_endl;

void TFastoreServer::TConnection::init(SOCKET socket, TFastoreServer * server,	const sockaddr* addr, socklen_t addrLen)
{
	tSocket_->setSocketFD(INT_CAST(socket));
	tSocket_->setCachedAddress(addr, addrLen);

	server_ = server;
	appState_ = APP_INIT;

	readBufferPos_ = 0;
	readWant_ = 0;

	writeBuffer_ = NULL;
	writeBufferSize_ = 0;
	writeBufferPos_ = 0;
	largestWriteBufferSize_ = 0;

	socketState_ = SOCKET_RECV_FRAMING;
	callsForResize_ = 0;

	// get input/transports
	factoryInputTransport_ = server_->getInputTransportFactory()->getTransport(inputTransport_);
	factoryOutputTransport_ = server_->getOutputTransportFactory()->getTransport(outputTransport_);

	// Create protocol
	inputProtocol_ = server_->getInputProtocolFactory()->getProtocol(factoryInputTransport_);
	outputProtocol_ = server_->getOutputProtocolFactory()->getProtocol(factoryOutputTransport_);

	// Set up for any server event handler
	serverEventHandler_ = server_->getEventHandler();
	if (serverEventHandler_ != NULL)
	{
		connectionContext_ = serverEventHandler_->createContext(inputProtocol_,	outputProtocol_);
	}
	else
	{
		connectionContext_ = NULL;
	}

	// Get the processor
	processor_ = server_->getProcessor(inputProtocol_, outputProtocol_, tSocket_);

	lastAction_ = clock();
}

void TFastoreServer::TConnection::workSocket()
{
	lastAction_ = clock();

	int got=0, left=0, sent=0;
	uint32_t fetch = 0;


	switch (socketState_)
	{
		case SOCKET_RECV_FRAMING:
			union
			{
				uint8_t buf[sizeof(uint32_t)];
				uint32_t size;
			} framing;

			// if we've already received some bytes we kept them here
			framing.size = readWant_;
			// determine size of this frame
			try
			{
				// Read from the socket
				fetch = tSocket_->read(&framing.buf[readBufferPos_], uint32_t(sizeof(framing.size) - readBufferPos_));
				if (fetch == 0)
				{
					// Whenever we get here it means a remote disconnect
					close();
					return;
				}
				readBufferPos_ += fetch;
			}
			catch (TTransportException& te)
			{
				GlobalOutput.printf("TConnection::workSocket(): %s", te.what());
				close();

				return;
			}

			if (readBufferPos_ < sizeof(framing.size))
			{
				// more needed before frame size is known -- save what we have so far
				readWant_ = framing.size;
				return;
			}

			readWant_ = ntohl(framing.size);
			if (readWant_ > server_->getMaxFrameSize())
			{
				// Don't allow giant frame sizes.  This prevents bad clients from
				// causing us to try and allocate a giant buffer.
				ostringstream msg;
				msg << "TFastoreServer: frame size too large "
					<< "("  << readWant_ << " > " << server_->getMaxFrameSize() << ") "
					<< "from client " << tSocket_->getSocketInfo() << ". "
					<< "Remote side not using TFramedTransport?",
				GlobalOutput.printf( "%s\n", msg.str().c_str() );
				close();
				return;
			}
			// size known; now get the rest of the frame
			transition();
			return;

		case SOCKET_RECV:
			// It is an error to be in this state if we already have all the data
			assert(readBufferPos_ < readWant_);
			try 
			{
				// Read from the socket
				fetch = readWant_ - readBufferPos_;
				got = tSocket_->read(readBuffer_ + readBufferPos_, fetch);
			}
			catch (TTransportException& te)
			{
				GlobalOutput.printf("TConnection::workSocket(): %s", te.what());
				close();

				return;
			}

			if (got > 0)
			{
				// Move along in the buffer
				readBufferPos_ += got;

				// Check that we did not overdo it
				assert(readBufferPos_ <= readWant_);

				// We are done reading, move onto the next state
				if (readBufferPos_ == readWant_)
				{
					transition();
				}
				return;
			}

			// Whenever we get down here it means a remote disconnect
			close();

			return;

		case SOCKET_SEND:
			// Should never have position past size
			assert(writeBufferPos_ <= writeBufferSize_);

			// If there is no data to send, then let us move on
			if (writeBufferPos_ == writeBufferSize_) {
				GlobalOutput("WARNING: Send state with no data to send\n");
				transition();
				return;
			}

			try 
			{
				left = writeBufferSize_ - writeBufferPos_;
				sent = tSocket_->write_partial(writeBuffer_ + writeBufferPos_, left);
			}
			catch (TTransportException& te) 
			{
				GlobalOutput.printf("TConnection::workSocket(): %s ", te.what());
				close();
				return;
			}

			writeBufferPos_ += sent;

			// Did we overdo it?
			assert(writeBufferPos_ <= writeBufferSize_);

			// We are done!
			if (writeBufferPos_ == writeBufferSize_)
			{
				transition();
			}

			return;

		default:
			GlobalOutput.printf("Unexpected Socket State %d", socketState_);
			assert(0);
	}
}

/**
* This is called when the application transitions from one state into
* another. This means that it has finished writing the data that it needed
* to, or finished receiving the data that it needed to.
*/
void TFastoreServer::TConnection::transition()
{
	// ensure this connection is active right now
	assert(server_);

	// Switch upon the state that we are currently in and move to a new state
	switch (appState_)
	{
		case APP_READ_REQUEST:
			// We are done reading the request, package the read buffer into transport
			// and get back some data from the dispatch function
			inputTransport_->resetBuffer(readBuffer_, readBufferPos_);
			outputTransport_->resetBuffer();
			// Prepend four bytes of blank space to the buffer so we can
			// write the frame size there later.
			outputTransport_->getWritePtr(4);
			outputTransport_->wroteBytes(4);

			try
			{
				// Invoke the processor
				processor_->process(inputProtocol_, outputProtocol_, this);
			} 
			catch (const TTransportException &ttx)
			{
				GlobalOutput.printf("TFastoreServer transport error in process(): %s", ttx.what());
				close();
				return;
			} 
			catch (const std::exception &x)
			{
				GlobalOutput.printf("Server::process() uncaught exception: %s: %s",	typeid(x).name(), x.what());
				close();
				return;
			} 
			catch (...)
			{
				GlobalOutput.printf("Server::process() unknown exception");
				close();
				return;
			}

			//The processor put us in parked state. We'll resume at the next case on the next call to transition.
			if(appState_ == APP_PARKED)
				return;

			// Intentionally fall through here, the call to process has written into
			// the writeBuffer_

		case APP_PARKED:
			// We have now finished processing a task and the result has been written
			// into the outputTransport_, so we grab its contents and place them into
			// the writeBuffer_ for actual writing by the libevent thread

			// Get the result of the operation
			outputTransport_->getBuffer(&writeBuffer_, &writeBufferSize_);

			// If the function call generated return data, then move into the send
			// state and get going
			// 4 bytes were reserved for frame size
			if (writeBufferSize_ > 4)
			{
				// Move into write state
				writeBufferPos_ = 0;
				socketState_ = SOCKET_SEND;

				// Put the frame size into the write buffer
				int32_t frameSize = (int32_t)htonl(writeBufferSize_ - 4);
				memcpy(writeBuffer_, &frameSize, 4);

				// Socket into write mode
				appState_ = APP_SEND_RESULT;

				return;
			}

			// In this case, the request was oneway and we should fall through
			// right back into the read frame header state
			goto LABEL_APP_INIT;

		case APP_SEND_RESULT:
			// it's now safe to perform buffer size housekeeping.
			if (writeBufferSize_ > largestWriteBufferSize_)
			{
				largestWriteBufferSize_ = writeBufferSize_;
			}

			if (server_->getResizeBufferEveryN() > 0 && ++callsForResize_ >= server_->getResizeBufferEveryN())
			{
				checkIdleBufferMemLimit(server_->getIdleReadBufferLimit(), server_->getIdleWriteBufferLimit());
				callsForResize_ = 0;
			}

			// N.B.: We also intentionally fall through here into the INIT state!

	LABEL_APP_INIT:
		case APP_INIT:

			// Clear write buffer variables
			writeBuffer_ = NULL;
			writeBufferPos_ = 0;
			writeBufferSize_ = 0;

			// Into read4 state we go
			socketState_ = SOCKET_RECV_FRAMING;
			appState_ = APP_READ_FRAME_SIZE;

			readBufferPos_ = 0;

			return;

		case APP_READ_FRAME_SIZE:
			// We just read the request length
			// Double the buffer size until it is big enough
			if (readWant_ > readBufferSize_) 
			{
				if (readBufferSize_ == 0) 
				{
					readBufferSize_ = 1;
				}

				uint32_t newSize = readBufferSize_;
				while (readWant_ > newSize)
				{
					newSize *= 2;
				}

				uint8_t* newBuffer = (uint8_t*)std::realloc(readBuffer_, newSize);
				if (newBuffer == NULL)
				{
					// nothing else to be done...
					throw std::bad_alloc();
				}

				readBuffer_ = newBuffer;
				readBufferSize_ = newSize;
			}

			readBufferPos_= 0;

			// Move into read request state
			socketState_ = SOCKET_RECV;
			appState_ = APP_READ_REQUEST;

			return;

		case APP_CLOSE_CONNECTION:
			close();
			return;

		default:
			GlobalOutput.printf("Unexpected Application State %d", appState_);
			assert(0);
	}
}

/**
* Closes a connection
*/
void TFastoreServer::TConnection::close()
{

	if (serverEventHandler_ != NULL)
	{
		serverEventHandler_->deleteContext(connectionContext_, inputProtocol_, outputProtocol_);
	}

	int id = tSocket_->getSocketFD();

	// Close the socket
	tSocket_->close();

	// close any factory produced transports
	factoryInputTransport_->close();
	factoryOutputTransport_->close();

	// Tell the server to free this connection when it can.
	server_->markForClose(id);
}

void TFastoreServer::TConnection::checkIdleBufferMemLimit(size_t readLimit,	size_t writeLimit)
{
	if (readLimit > 0 && readBufferSize_ > readLimit)
	{
		free(readBuffer_);
		readBuffer_ = NULL;
		readBufferSize_ = 0;
	}

	if (writeLimit > 0 && largestWriteBufferSize_ > writeLimit)
	{
		// just start over
		outputTransport_->resetBuffer(server_->getWriteBufferDefaultSize());
		largestWriteBufferSize_ = 0;
	}
}

/**
* TFastoreServer
*/
TFastoreServer::~TFastoreServer() 
{
	//Kill our current active connections...
	for (auto iter = activeConnections_.begin(); iter != activeConnections_.end(); ++iter)
	{		
		iter->second->forceClose();
	}

	closeMarkedConnections();

	// Clean up unused TConnection objects in connectionStack_
	while (!connectionPool_.empty())
	{
		TConnection* connection = connectionPool_.top();
		connectionPool_.pop();
		delete connection;
	}

	if (serverSocket_ >= 0)
	{
		if (0 != ::close(serverSocket_))
		{
			GlobalOutput.perror("TFastoreServer listenSocket_ close(): ", errno);
		}
		serverSocket_ = TFastoreServer::INVALID_SOCKET_VALUE;
	}

	if (_fds != NULL)
		delete _fds;

	
}

/**
* Creates a new connection either by reusing an object off the stack or
* by allocating a new one entirely
*/
TFastoreServer::TConnection* TFastoreServer::
	createConnection(SOCKET socket, const sockaddr* addr, socklen_t addrLen)
{
	// Check the connection stack to see if we can re-use
	TConnection* result = NULL;
	if (connectionPool_.empty())
	{
		result = new TConnection(socket, this, addr, addrLen);
	} 
	else
	{
		result = connectionPool_.top();
		connectionPool_.pop();
		result->init(socket, this, addr, addrLen);
	}

	activeConnections_.insert(pair<SOCKET,TConnection*>(socket, result));

	return result;
}

/**
* Marks a connection for closure at next opportunity
*/
void TFastoreServer::markForClose(int socket)
{
	closePool_.push_back(socket);
}

void TFastoreServer::closeMarkedConnections()
{
	//clean any dead connections discovered by socket id;
  for (size_t i = 0; i < closePool_.size(); ++i)
	{
		returnConnection(closePool_[i]);
	}

	closePool_.clear();
}

/**
* Returns a connection to the stack
*/
void TFastoreServer::returnConnection(int socket)
{

	auto it = activeConnections_.find(socket);
	if( it == activeConnections_.end() ) {
		ostringstream msg;
		msg << __func__ << ": no connection for socket " << socket;
		Log << log_err << msg.str() << log_endl;
		throw logic_error(msg.str());
	}
	TConnection* connection = it->second;
	activeConnections_.erase(it);

	if (connectionPoolLimit_ &&	(connectionPool_.size() >= connectionPoolLimit_))
	{
		delete connection;
	} 
	else
	{
		connection->checkIdleBufferMemLimit(idleReadBufferLimit_, idleWriteBufferLimit_);
		connectionPool_.push(connection);
	}
}

/**
* Creates a socket to listen on and binds it to the local port.
*/
void TFastoreServer::createAndListenOnSocket()
{
	SOCKET s;

	struct addrinfo hints, *res, *res0;
	int error;

	char port[sizeof("65536") + 1];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	sprintf(port, "%d", port_);

	// Wildcard address
	error = getaddrinfo(NULL, port, &hints, &res0);
	if (error)
	{
		throw TException("TFastoreServer::serve() getaddrinfo " + string(gai_strerror(error)));
	}

	// Pick the ipv6 address first since ipv4 addresses can be mapped
	// into ipv6 space.
	for (res = res0; res; res = res->ai_next)
	{
		if (res->ai_family == AF_INET6 || res->ai_next == NULL)
			break;
	}

	// Create the server socket
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (s == -1)
	{
		freeaddrinfo(res0);
		throw TException("TFastoreServer::serve() socket() -1");
	}

#ifdef IPV6_V6ONLY
	if (res->ai_family == AF_INET6)
	{
		int zero = 0;
		if (-1 == setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, const_cast_sockopt(&zero), sizeof(zero)))
		{
			GlobalOutput("TServerSocket::listen() IPV6_V6ONLY");
		}
	}
#endif // #ifdef IPV6_V6ONLY

	int one = 1;

	// Set reuseaddr to avoid 2MSL delay on server restart
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, const_cast_sockopt(&one), sizeof(one));

	if (::bind(s, res->ai_addr, static_cast<int>(res->ai_addrlen)) == -1)
	{
		::close(s);
		freeaddrinfo(res0);
		throw TTransportException(TTransportException::NOT_OPEN, "TFastoreServer::serve() bind", errno);
	}

	// Done with the addr info
	freeaddrinfo(res0);

	// Set up this file descriptor for listening
	listenSocket(s);
}

static int fcntl(SOCKET fd, int cmd, int flags)
{
	return ::fcntl(static_cast<int>(fd), cmd, flags);
}
/**
* Takes a socket created by listenSocket() and sets various options on it
* to prepare for use in the server.
*/
void TFastoreServer::listenSocket(SOCKET s)
{
	// Set socket to nonblocking mode
	int flags;
	if ((flags = fcntl(s, F_GETFL, 0)) < 0 || fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0)
	{
			::close(s);
			throw TException("TFastoreServer::serve() O_NONBLOCK");
	}

	int one = 1;
	struct linger ling = {0, 0};

	// Keepalive to ensure full result flushing
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, const_cast_sockopt(&one), sizeof(one));

	// Turn linger off to avoid hung sockets
	setsockopt(s, SOL_SOCKET, SO_LINGER, const_cast_sockopt(&ling), sizeof(ling));

	// Set TCP nodelay if available, MAC OS X Hack
	// See http://lists.danga.com/pipermail/memcached/2005-March/001240.html
#ifndef TCP_NOPUSH
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, const_cast_sockopt(&one), sizeof(one));
#endif

#ifdef TCP_LOW_MIN_RTO
	if (TSocket::getUseLowMinRto())
	{
		setsockopt(s, IPPROTO_TCP, TCP_LOW_MIN_RTO, const_cast_sockopt(&one), sizeof(one));
	}
#endif

	if (listen(s, LISTEN_BACKLOG) == -1)
	{
		::close(s);
		throw TException("TFastoreServer::serve() listen");
	}

	// Cool, this socket is good to go, set it as the serverSocket_
	serverSocket_ = s;
}

bool  TFastoreServer::serverOverloaded()
{
	size_t activeConnections = activeConnections_.size();
	if (activeConnections > maxConnections_)
	{
		if (!overloaded_)
		{
			GlobalOutput.printf("TFastoreServer: overload condition begun.");
			overloaded_ = true;
		}
	} 
	else
	{
	  if (overloaded_ && activeConnections <= SAFE_CAST(size_t, overloadHysteresis_ * double(maxConnections_)))
		{
			GlobalOutput.printf("TFastoreServer: overload ended; %u dropped (%llu total)", nConnectionsDropped_, nTotalConnectionsDropped_);
			nConnectionsDropped_ = 0;
			overloaded_ = false;
		}
	}

	return overloaded_;
}

void TFastoreServer::stop()
{
	_stop = true;
}

void TFastoreServer::shutdown()
{
	_shutdown = true;
	_shutdownStart = clock();
}

bool TFastoreServer::isShuttingDown()
{
	return _shutdown;
}

void TFastoreServer::expireConnections()
{
	if (connectionExpireTime_ > 0)
	{
		for (auto iter = activeConnections_.begin(); iter != activeConnections_.end(); ++iter)
		{
			iter->second->expire(connectionExpireTime_);
		}
	}
}

void TFastoreServer::serve()
{
	// init listen socket
	createAndListenOnSocket();

	// Notify handler of the preServe event
	if (eventHandler_ != NULL) 
	{
		eventHandler_->preServe();
	}

	run();
}

//TODO: Consider refactoring this into a state machine similar to what the connections are using.
// right now theres a bit of logic here and there scattered around to handle these cases.
//
// STARTUP
// SHUTDOWN
// STOP
// RUN
// OVERLOADED
// TRANSITION (Used to handle the case when the connection limit is lowered. May be able to reused overloaded state).
void TFastoreServer::run()
{

	//TODO: Have this reallocate when max connection size changes.
	//Since max connection size is likely to change due to a call on one of the connections,
	//we must not actually resize until all connections are finished processing.
	//Also, if we reduce the number of max connections, we need a way to either terminate all the active
	//connections, or only reduce the size of the fds array once the connections are finished, since we'll have no
	//way to poll active connections if they don't have a spot in the fd array.
	_fds = new pollfd[getMaxConnections() + 1];

	_fds[0].events = POLLIN;
	_fds[0].fd = serverSocket_;	

	Log << log_info << "TFastoreServer running" << log_endl;
	while(!_stop)
	{
		_fds[0].revents = 0;

		//construct fd array using active connections
		int fdindex = 1;
		for (auto iter = activeConnections_.begin(); iter != activeConnections_.end(); )
		{
			TAppState connState = iter->second->getState();
			switch (connState)
			{
				case APP_READ_FRAME_SIZE:
				case APP_READ_REQUEST:
					_fds[fdindex].events = POLLIN;
					_fds[fdindex].fd = iter->first;
					_fds[fdindex].revents = 0;
					++fdindex;
					break;

				case APP_SEND_RESULT:
					_fds[fdindex].events = POLLOUT;
					_fds[fdindex].fd = iter->first;
					_fds[fdindex].revents = 0;
					++fdindex;
					break;

				case APP_CLOSE_CONNECTION:
				case APP_INIT:
					//The transition call will cause new connections to be initialized
					//and closed connection to be added to the closePool.
					iter->second->transition();
					break;
				case APP_PARKED:
				default:
					break;
			}

			++iter;
		}

#ifndef _WIN32
		// Signal the parent process we've successfully set up. 
		kill(getppid(), SIGUSR1);
#endif
		int numready = poll(_fds, fdindex, pollTimeout_);
		if (numready == -1)
		{
			GlobalOutput.printf("TFastoreServer: poll error: %s", 
								strerror(errno));
		}
		else if(numready > 0 )
		{
			for (int i = 1; i < fdindex; i++)
			{
				if (_fds[i].revents != 0)
					activeConnections_[_fds[i].fd]->workSocket();
			}

			//We still accept connections even if we are shutting down
			//So that we can tell them we are shutting down (in the future, not implemented yet...)
			// (Do we want a different behavior?)
			if (_fds[0].revents != 0)
			{
				acceptConnections();
			}
		}

		//Close connections that have been unused for a while.
		expireConnections();

		//If any active connections were marked for closure on thise last iteration,
		//free them and return them to the pool.
		closeMarkedConnections();

		//If we are passed our timeout, or have successfully closed all connections, then stop serving.
		if (_shutdown &&  /*(clock() - _shutdownStart > _shutdownTimeout || */ activeConnections_.size() == 0) //)
			stop();

		//For now, just close immediately
		//if (_shutdown)
		//	stop();
	}
	Log << log_info << "TFastoreServer stopped" << log_endl;

}

void TFastoreServer::acceptConnections()
{
	//Try to accept new connections
	socklen_t addrLen;
	sockaddr_storage addrStorage;
	sockaddr* addrp = (sockaddr*)&addrStorage;
	addrLen = sizeof(addrStorage);

	// Going to accept a new client socket
	SOCKET clientSocket;

	// Accept as many new clients as possible, even though libevent signaled only
	// one, this helps us to avoid having to go back into the libevent engine so
	// many times
	while ((clientSocket = ::accept(serverSocket_, addrp, &addrLen)) != -1)
	{
		// If we're overloaded, take action here
		if (overloadAction_ != T_OVERLOAD_NO_ACTION && serverOverloaded())
		{
			nConnectionsDropped_++;
			nTotalConnectionsDropped_++;

			::close(clientSocket);
			break;
		}

		// Explicitly set this socket to NONBLOCK mode
		int flags;
		if ((flags = fcntl(clientSocket, F_GETFL, 0)) < 0 || fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			GlobalOutput.perror("TFastoreServer: set O_NONBLOCK (fcntl) ", errno);
			::close(clientSocket);

		}

		// Create a new TConnection for this client socket.
		TConnection* clientConnection =
			createConnection(clientSocket, addrp, addrLen);

		// Fail fast if we could not create a TConnection object
		if (clientConnection == NULL) {
			GlobalOutput.printf("TFastoreServer: failed TConnection factory");
			::close(clientSocket);
			break;
		}

		//Initialize connection, try to perform first work.
		clientConnection->transition();


		// addrLen is written by the accept() call, so needs to be set before the next call.
		addrLen = sizeof(addrStorage);
	}


	// Done looping accept, now we have to make sure the error is due to
	// blocking. Any other error is a problem
	if (errno != EAGAIN && errno != EWOULDBLOCK)
	{
		GlobalOutput.perror("TFastoreServer: accept() ", errno);
	}
}

}}} // apache::thrift::server
