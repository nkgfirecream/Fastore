//Might not be needed.. Still exploring..

#include <thrift/transport/TBufferTransports.h>
#include <functional>

using namespace apache::thrift::transport;

class TParkableTransport : public TBufferedTransport
{
	public:
	
	TParkableTransport(boost::shared_ptr<TTransport> transport); 
	TParkableTransport(boost::shared_ptr<TTransport> transport, uint32_t sz);
	TParkableTransport(boost::shared_ptr<TTransport> transport, uint32_t rsz, uint32_t wsz);

	void setParked(bool parked);
	bool getParked();
	int getId();
	void open();
	bool isOpen();
	bool peek();
	void close();
	void flush();
	boost::shared_ptr<TTransport> getUnderlyingTransport();
	uint32_t readAll(uint8_t* buf, uint32_t len);
	uint32_t read(uint8_t* buf, uint32_t len);
	const uint8_t* borrow(uint8_t* buf, uint32_t* len);
	void consume(uint32_t len);
	void write(uint8_t* buf, uint32_t len);

	private:
	
	bool _parked;
};


class TParkableTransportFactory : public TTransportFactory {
 public:
  TParkableTransportFactory() {}

  virtual ~TParkableTransportFactory() {}

  /**
   * Wraps the transport into a buffered one.
   */
  virtual boost::shared_ptr<TTransport> getTransport(boost::shared_ptr<TTransport> trans) {
    return boost::shared_ptr<TTransport>(new TParkableTransport(trans));
  }

};

