#pragma once
#include <string>
#include <algorithm>
#include "../FastoreCore/safe_cast.h"

template<typename charT>
class CaseInsensitiveComparer
{
    const std::locale& _locale;
public:
    CaseInsensitiveComparer(const std::locale& loc) : _locale(loc) {}
    bool operator()(charT ch1, charT ch2) 
	{
        return std::toupper(ch1, _locale) == std::toupper(ch2, _locale);
    }
};

template<typename T>
int insensitiveStrPos(const T& str1, const T& str2, const std::locale& locale = std::locale() )
{
    typename T::const_iterator it = std::search(str1.begin(), str1.end(), 
						str2.begin(), str2.end(), 
						CaseInsensitiveComparer<typename T::value_type>(locale));
    if (it != str1.end()) 
		return SAFE_CAST(int, it - str1.begin());
    else 
		return -1;
}