#include "TParkableTransport.h"

TParkableTransport::TParkableTransport(boost::shared_ptr<TTransport> transport)
	: TBufferedTransport(transport), _parked(false) {}

TParkableTransport::TParkableTransport(boost::shared_ptr<TTransport> transport, uint32_t sz)
	: TBufferedTransport(transport, sz), _parked(false) {}

TParkableTransport::TParkableTransport(boost::shared_ptr<TTransport> transport, uint32_t rsz, uint32_t wsz)
	: TBufferedTransport(transport, rsz, wsz), _parked(false) {}

void TParkableTransport::setParked(bool parked)
{
	_parked = parked;
}

bool TParkableTransport::getParked()
{
	return _parked;
}

int TParkableTransport::getId()
{
	//Some id to distinguish connections
	return 0;
}

void TParkableTransport::open()
{ 
	if(!_parked)
		TBufferedTransport::open();
}

bool TParkableTransport::isOpen()
{
	return TBufferedTransport::isOpen();
}

bool TParkableTransport::peek()
{ 
	return TBufferedTransport::peek();
}

void TParkableTransport::close()
{ 
	if(!_parked)
		TBufferedTransport::close();
}

void TParkableTransport::flush()
{ 
	if(!_parked)
		TBufferedTransport::flush();
}

boost::shared_ptr<TTransport> TParkableTransport::getUnderlyingTransport()
{ 
	return TBufferedTransport::getUnderlyingTransport();
}

uint32_t TParkableTransport::readAll(uint8_t* buf, uint32_t len)
{
	if(!_parked)
		return TBufferedTransport::readAll(buf, len);

	return 0;
}

uint32_t TParkableTransport::read(uint8_t* buf, uint32_t len)
{
	if(!_parked)
		return TBufferedTransport::read(buf, len);

	return 0;
}

const uint8_t* TParkableTransport::borrow(uint8_t* buf, uint32_t* len)
{
	if(!_parked)
		return TBufferedTransport::borrow(buf, len);

	return NULL;
}

void TParkableTransport::consume(uint32_t len)
{
	if(!_parked)
		TBufferedTransport::consume(len);
}

void TParkableTransport::write(uint8_t* buf, uint32_t len)
{
	if(!_parked)
		return TBufferedTransport::write(buf, len);
}


