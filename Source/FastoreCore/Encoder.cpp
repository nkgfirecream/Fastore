#include "Encoder.h"

void Encoder::ReadString(const std::string& input, std::string& output)
{
	output.assign(input.data(), input.size());
}

void Encoder::WriteString(const std::string& input, std::string& output)
{
	output.assign(input.data(), input.size());
}

void Encoder::ReadBool(const std::string& input, bool& output)
{
	output = *(bool*)input.data();
}

void Encoder::WriteBool(const bool& input, std::string& output)
{
	output.assign((char*)&input, sizeof(bool));
}

void Encoder::ReadLong(const std::string& input, long long& output)
{
	output = *(long long*)input.data();
}

void Encoder::WriteLong(const long long& input, std::string& output)
{
	output.assign((char*)&input, sizeof(long long));
}

void Encoder::ReadInt(const std::string& input, int& output)
{
	output = *(int*)input.data();
}

void Encoder::WriteInt(const int& input, std::string& output)
{
	output.assign((char*)&input, sizeof(int));
}


