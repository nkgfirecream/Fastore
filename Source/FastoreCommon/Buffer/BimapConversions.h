#pragma once
#include <string>

template<typename T>
std::string GetStringFromValue(const T& value)
{
	std::string temp;
	temp.assign((const char*)&value, sizeof(T));
	return temp;
}

template<typename T>
T GetValueFromString(const std::string& string)
{
	return *(T*)string.data();
}

template<>
std::string GetStringFromValue<std::string>(const std::string& value)
{
	return value;
}

template<>
std::string GetValueFromString<std::string>(const std::string& string)
{
	return string;	
}