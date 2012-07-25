#ifndef _THRIFT_TRANSPORT_TFASTOREFILETRANSPORT_H_
#define _THRIFT_TRANSPORT_TFASTOREFILETRANSPORT_H_ 1

#include <string>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <thrift\transport\TTransport.h>
#include <thrift\transport\TVirtualTransport.h>

namespace apache { namespace thrift { namespace transport {

/**
 * This transport operates on a file pointer that only supports reading an existing file,
 * or creating/overwriting a file. There is no seeking or appending.
 */ 
class TFastoreFileTransport : public TVirtualTransport<TFastoreFileTransport>
{
public:

	TFastoreFileTransport(std::string filename, bool read)
	: _filename(filename), _read(read), _file(NULL)
	{ }

	~TFastoreFileTransport()
	{
		close();
	}

	bool isOpen();

	void open();

	void close();

	bool peek();

	void flush();

	uint32_t read(uint8_t* buf, uint32_t len);

	void write(const uint8_t* buf, uint32_t len);

protected:
	FILE* _file;
	std::string _filename;
	long _filesize;
	bool _read;
};

}}} // apache::thrift::transport

#endif // #ifndef _THRIFT_TRANSPORT_TFASTOREFILETRANSPORT_H_
