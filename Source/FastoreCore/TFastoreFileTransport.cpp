#include <cerrno>
#include <sstream>
#include <stdexcept>

#include "TFastoreFileTransport.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

#define SAFE_CAST(t,f) safe_cast(__FILE__, __LINE__, (t), (f))
#define SHORT_CAST(x) safe_cast(__FILE__, __LINE__, short(), (x))

template <typename T, typename F>
T safe_cast(const char file[], size_t line, T, F input) {
  using std::numeric_limits;
  std::ostringstream msg;

  if( numeric_limits<F>::is_signed && !numeric_limits<T>::is_signed ) {
    if( input < 0 ) {
      msg << file << ":" << line << ": " 
	  << "signed value " << input << " cannot be cast to unsigned type";
      throw std::runtime_error(msg.str());
    }
  }
  if( numeric_limits<T>::max() < static_cast<size_t>(input) ) {
    msg << file << ":" << line << ": " 
	<< input << ", size " << sizeof(F) 
	<< ", cannot be cast to unsigned type of size" << sizeof(T);
    throw std::runtime_error(msg.str());
  }
  return static_cast<T>(input);
}

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
		//TODO: Real error handling.
		throw TTransportException(TTransportException::UNKNOWN, "TFastoreFileTransport::close()");
	}
}

uint32_t TFastoreFileTransport::read(uint8_t* buf, uint32_t len)
{
	if (_read)
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
				throw TTransportException(TTransportException::UNKNOWN, "FastoreFileTransport::read()", errno);
			}

			return SAFE_CAST(uint32_t(), rv);
		}
	}
	else
		throw TTransportException(TTransportException::UNKNOWN, "Attempted read on write-only TFastoreFileTransport");
}

void TFastoreFileTransport::write(const uint8_t* buf, uint32_t len)
{

  if (_read)
	{

		flush();

		while (len > 0)
		{
			size_t rv = ::fwrite(buf, 1, len, _file);

			if (rv < 0)
			{
				int errno_copy = errno;
				throw TTransportException(TTransportException::UNKNOWN, "TFastoreFileTransport::write()", errno_copy);
			} 
			if (rv == 0)
			{
				throw TTransportException(TTransportException::END_OF_FILE, "TFastoreFileTransport::write()");
			}

			buf += rv;
			// rv <= len, because len was input to write(2)
			len -= static_cast<uint32_t>(rv);
		}

		//For debugging purposes, flush after every write.
		flush();
	}
	else
		throw TTransportException(TTransportException::UNKNOWN, "Attempted write on read-only TFastoreFileTransport");
}

void TFastoreFileTransport::flush()
{
	if (isOpen())
		fflush(_file);
}

bool TFastoreFileTransport::peek()
{
	//4 bytes is the smallest object thrift will write.
	//Probably, if we only have 3 or less bytes remaining 
	//there's some sort of error condition.
	return (isOpen() && _filesize - ftell(_file) > 0);
}

void TFastoreFileTransport::open()
{
	try
	{
		_file = fopen(_filename.c_str(), _read ? "rb" : "wb");
		//Perhaps I could just just the filesize with fstat?
		fseek(_file, 0, SEEK_END);

		//Get filesize
		_filesize = ftell(_file);

		//Return to position
		fseek(_file, 0, SEEK_SET);  
	}
	catch(...)
	{
		//TODO: Real error handling if we can't acquire the file.
		throw TTransportException(TTransportException::UNKNOWN, "TFastoreFileTransport::open()");
	}
}

}}} // apache::thrift::transport
