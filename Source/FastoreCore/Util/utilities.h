#pragma once
#include "../typedefs.h"

inline int boolToNormalizedInt(bool forward)
{
	return forward * 2 - 1;
}

inline int boolToInt(bool forward)
{
	return forward * forward;
}

inline bool readBool(const std::string item)
{
	const char* buf = item.data();
	bool toReturn;

	memcpy(&toReturn, buf, sizeof(bool));
	return toReturn;
}

inline std::string writeBool(bool& item)
{
	char* buffer = new char[sizeof(bool)];
	memcpy(buffer, &item, sizeof(bool));

	std::string toReturn(buffer, sizeof(bool));

	delete buffer;

	return toReturn;
}

inline int readInt(const std::string item)
{
	const char* buf = item.data();
	int toReturn;

	memcpy(&toReturn, buf, sizeof(int));
	return toReturn;
}

inline std::string writeInt(int& item)
{
	char* buffer = new char[sizeof(int)];
	memcpy(buffer, &item, sizeof(int));

	std::string toReturn(buffer, sizeof(int));

	delete buffer;

	return toReturn;
}

inline long long readLong(const std::string& item)
{
	const char* buf = item.data();
	long long toReturn;

	memcpy(&toReturn, buf, sizeof(long long));
	return toReturn;
}

inline std::string writeLong(long long& item)
{
	char* buffer = new char[sizeof(long long)];
	memcpy(buffer, &item, sizeof(long long));

	std::string toReturn(buffer, sizeof(long long));

	delete buffer;

	return toReturn;
}

inline std::string readString(const std::string& item)
{
	const char* buf = item.data();
	unsigned int size;
	memcpy(&size, buf, sizeof(unsigned int));

	std::string toReturn;
	if (size == 0)
		return toReturn;

	toReturn.assign(buf + sizeof(unsigned int), size);

	return toReturn;
}

inline std::string writeString(std::string& item)
{
	unsigned int size = item.size();

	char* buffer = new char[size + sizeof(unsigned int)];
	
	memcpy(buffer, &size, sizeof(unsigned int));

	if (size != 0)
	{
		const char* data = item.data();
		memcpy(buffer + sizeof(unsigned int), data, size);
	}

	std::string toReturn(buffer, size + sizeof(unsigned int));

	delete buffer;

	return toReturn;
}
