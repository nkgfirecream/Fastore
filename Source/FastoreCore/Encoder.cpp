#include "Encoder.h"

Encoder* Encoder::_encoder;

Encoder::Encoder()
{
	_buffer = new char[1024];
}

Encoder* Encoder::GetEncoder()
{
	if (_encoder == NULL)
		_encoder = new Encoder();

	return _encoder;
}

void Encoder::ReadString(const std::string& input, std::string& output)
{
	const char* buf = input.data();
	unsigned int size = *(int*)buf;

	output.assign(buf + sizeof(unsigned int), size);
}

void Encoder::WriteString(const std::string& input, std::string& output)
{
	unsigned int size = input.size();

	memcpy(_buffer, &size, sizeof(unsigned int));
	memcpy(_buffer + sizeof(unsigned int), input.data(), size);

	output.assign(_buffer, size + sizeof(unsigned int));
}

void Encoder::ReadBool(const std::string& input, bool& output)
{
	output = *(bool*)input.data();
}

void Encoder::WriteBool(const bool& input, std::string& output)
{
	memcpy(_buffer, &input, sizeof(bool));
	output.assign(_buffer, sizeof(bool));
}

void Encoder::ReadLong(const std::string& input, long long& output)
{
	output = *(long long*)input.data();
}

void Encoder::WriteLong(const long long& input, std::string& output)
{
	memcpy(_buffer, &input, sizeof(long long));
	output.assign(_buffer, sizeof(long long));
}

void Encoder::ReadInt(const std::string& input, int& output)
{
	output = *(int*)input.data();
}

void Encoder::WriteInt(const int& input, std::string& output)
{
	memcpy(_buffer, &input, sizeof(int));
	output.assign(_buffer, sizeof(int));
}


