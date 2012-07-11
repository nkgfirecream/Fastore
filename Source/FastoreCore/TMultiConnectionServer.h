
#ifndef _THRIFT_SERVER_TMULTICONNECTIONSERVER_H_
#define _THRIFT_SERVER_TMULTICONNECTIONSERVER_H_ 1

#include <thrift/server/TServer.h>
#include <thrift/transport/TServerTransport.h>

using namespace apache::thrift;
using namespace apache::thrift::server;

class TMultiConnectionServer : public TServer {
 public:
  template<typename ProcessorFactory>
  TMultiConnectionServer(
      const boost::shared_ptr<ProcessorFactory>& processorFactory,
      const boost::shared_ptr<TServerTransport>& serverTransport,
      const boost::shared_ptr<TTransportFactory>& transportFactory,
      const boost::shared_ptr<TProtocolFactory>& protocolFactory,
      THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)) :
    TServer(processorFactory, serverTransport, transportFactory,
            protocolFactory),
    stop_(false) {}

  template<typename Processor>
  TMultiConnectionServer(
      const boost::shared_ptr<Processor>& processor,
      const boost::shared_ptr<TServerTransport>& serverTransport,
      const boost::shared_ptr<TTransportFactory>& transportFactory,
      const boost::shared_ptr<TProtocolFactory>& protocolFactory,
      THRIFT_OVERLOAD_IF(Processor, TProcessor)) :
    TServer(processor, serverTransport, transportFactory, protocolFactory),
    stop_(false) {}

  template<typename ProcessorFactory>
  TMultiConnectionServer(
      const boost::shared_ptr<ProcessorFactory>& processorFactory,
      const boost::shared_ptr<TServerTransport>& serverTransport,
      const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
      const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
      const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
      const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
      THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)) :
    TServer(processorFactory, serverTransport,
            inputTransportFactory, outputTransportFactory,
            inputProtocolFactory, outputProtocolFactory),
    stop_(false) {}

  template<typename Processor>
  TMultiConnectionServer(
      const boost::shared_ptr<Processor>& processor,
      const boost::shared_ptr<TServerTransport>& serverTransport,
      const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
      const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
      const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
      const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
      THRIFT_OVERLOAD_IF(Processor, TProcessor)) :
    TServer(processor, serverTransport,
            inputTransportFactory, outputTransportFactory,
            inputProtocolFactory, outputProtocolFactory),
    stop_(false) {}

  ~TMultiConnectionServer() {}

  void serve();

  void stop() {
    stop_ = true;
    serverTransport_->interrupt();
  }

  //Id is a place holder for
  void parkConnection(int id);
  void resumeConnection(int id);

 protected:
  bool stop_;

};

#endif // #ifndef _THRIFT_SERVER_TMULTICONNECTIONSERVER_H_
