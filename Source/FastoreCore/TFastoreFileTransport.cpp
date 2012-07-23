#include <cerrno>
#include <exception>
//#include <sys\stat.h>

#include "TFastoreFileTransport.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;

namespace apache { namespace thrift { namespace transport {

bool TFastoreFileTransport::isOpen()
{
	return _file != NULL;
}

void TFastoreFileTransport::close()
{
	  if (!isOpen())
	  {
		return;
	  }

	  try
	  {
		  fflush(_file);
		  fclose(_file);
		  _file = NULL;
	  }
	  catch(...)
	  {
			throw TTransportException(TTransportException::UNKNOWN, "TFastoreFileTransport::close()");
	  }
}

uint32_t TFastoreFileTransport::read(uint8_t* buf, uint32_t len)
{
	  unsigned int maxRetries = 5; // same as the TSocket default
	  unsigned int retries = 0;
	  while (true)
	  {
			size_t rv = ::fread(buf, 1, len, _file);
			if (rv < 0)
			{
				  if (errno == EINTR && retries < maxRetries) 
				  {
					// If interrupted, try again
					++retries;
					continue;
				  }
				  int errno_copy = errno;
				  throw TTransportException(TTransportException::UNKNOWN, "FastoreFileTransport::read()", errno_copy);
			}
			return rv;
	  }
}

void TFastoreFileTransport::write(const uint8_t* buf, uint32_t len)
{
	  while (len > 0)
	  {
			size_t rv = ::fwrite(buf, 1, len, _file);

			if (rv < 0)
			{
			  int errno_copy = errno;
			  throw TTransportException(TTransportException::UNKNOWN, "TFastoreFileTransport::write()", errno_copy);
			} 
			else if (rv == 0)
			{
			  throw TTransportException(TTransportException::END_OF_FILE, "TFastoreFileTransport::write()");
			}

			buf += rv;
			len -= rv;
	  }
}

bool TFastoreFileTransport::peek()
{
	//4 bytes is the smallest object thrift will write.
	//Probably, if we only have 3 or less bytes remaining 
	//there's some sort of error condition.
	return (isOpen() && _filesize - ftell(_file) >= 4);
}

}}} // apache::thrift::transport
