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
 * Dead-simple wrapper around a file descriptor.
 *
 */
class TFastoreFileTransport : public TVirtualTransport<TFastoreFileTransport>
{
public:

	TFastoreFileTransport(FILE* file)
	: _file(file)
	{
		//Remeber current position in file..
		fpos_t curpos;
		fgetpos(_file, &curpos);
		fseek(_file, 0, SEEK_END);

		//Get filesize
		_filesize = ftell(_file);

		//Return to position
		fseek(_file, curpos, SEEK_SET);  
	}

	~TFastoreFileTransport()
	{
		close();
	}

	bool isOpen();

	void open() { /*Intentionally do nothing */}

	void close();

	bool peek();

	uint32_t read(uint8_t* buf, uint32_t len);

	void write(const uint8_t* buf, uint32_t len);

protected:
	FILE* _file;

	long _filesize;
};

}}} // apache::thrift::transport

#endif // #ifndef _THRIFT_TRANSPORT_TFASTOREFILETRANSPORT_H_
