
#include "TMultiConnectionServer.h"
#include "ConnectionContext.h"
#include <thrift/transport/TTransportException.h>
#include <string>
#include <iostream>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

void TMultiConnectionServer::serve() {

	boost::shared_ptr<TTransport> client;
	boost::shared_ptr<TTransport> inputTransport;
	boost::shared_ptr<TTransport> outputTransport;
	boost::shared_ptr<TProtocol> inputProtocol;
	boost::shared_ptr<TProtocol> outputProtocol;

	// Start the server listening
	serverTransport_->listen();

	// Fetch client from server
	while (!stop_) {
		try {
			client = serverTransport_->accept();
			inputTransport = inputTransportFactory_->getTransport(client);
			outputTransport = outputTransportFactory_->getTransport(client);
			inputProtocol = inputProtocolFactory_->getProtocol(inputTransport);
			outputProtocol = outputProtocolFactory_->getProtocol(outputTransport);
		} catch (TTransportException& ttx) {
			if (inputTransport != NULL) { inputTransport->close(); }
			if (outputTransport != NULL) { outputTransport->close(); }
			if (client != NULL) { client->close(); }
			if (!stop_ || ttx.getType() != TTransportException::INTERRUPTED) {
				string errStr = string("TServerTransport died on accept: ") + ttx.what();
				GlobalOutput(errStr.c_str());
			}
			continue;
		} catch (TException& tx) {
			if (inputTransport != NULL) { inputTransport->close(); }
			if (outputTransport != NULL) { outputTransport->close(); }
			if (client != NULL) { client->close(); }
			string errStr = string("Some kind of accept exception: ") + tx.what();
			GlobalOutput(errStr.c_str());
			continue;
		} catch (string s) {
			if (inputTransport != NULL) { inputTransport->close(); }
			if (outputTransport != NULL) { outputTransport->close(); }
			if (client != NULL) { client->close(); }
			string errStr = string("Some kind of accept exception: ") + s;
			GlobalOutput(errStr.c_str());
			break;
		}

		// Get the processor
		boost::shared_ptr<TProcessor> processor = getProcessor(inputProtocol,
			outputProtocol, client);

		//context should go out of scope at the end of the loop and release the context unless an event happend and captured the context;
		boost::shared_ptr<TMultiConnectionContext>  context = boost::shared_ptr<TMultiConnectionContext>(new TMultiConnectionContext());
		context->client = client;
		context->inputProtocol = inputProtocol;
		context->outputProtocol = outputProtocol;
		context->inputTransport = inputTransport;
		context->outputTransport = outputTransport;

		void* connectionContext = &context;

		try {
			processor->process(inputProtocol, outputProtocol, connectionContext);
		} catch (const TTransportException& ttx) {
			string errStr = string("TMultiConnectionServer client died: ") + ttx.what();
			GlobalOutput(errStr.c_str());
		} catch (const std::exception& x) {
			GlobalOutput.printf("TMultiConnectionServer exception: %s: %s",
				typeid(x).name(), x.what());
		} catch (...) {
			GlobalOutput("TMultiConnectionServer uncaught exception.");
		}

		try {
			inputTransport->close();
		} catch (const TTransportException& ttx) {
			string errStr = string("TMultiConnectionServer input close failed: ")
				+ ttx.what();
			GlobalOutput(errStr.c_str());
		}
		try {
			outputTransport->close();
		} catch (const TTransportException& ttx) {
			string errStr = string("TMultiConnectionServer output close failed: ")
				+ ttx.what();
			GlobalOutput(errStr.c_str());
		}
		try {
			client->close();
		} catch (const TTransportException& ttx) {
			string errStr = string("TMultiConnectionServer client close failed: ")
				+ ttx.what();
			GlobalOutput(errStr.c_str());
		}
	}

	if (stop_) {
		try {
			serverTransport_->close();
		} catch (TTransportException &ttx) {
			string errStr = string("TServerTransport failed on close: ") + ttx.what();
			GlobalOutput(errStr.c_str());
		}
		stop_ = false;
	}
}
