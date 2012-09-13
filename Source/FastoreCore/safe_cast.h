//TODO: We need to decide whether to use include guards everywhere or simply rely on pragma directives.
#pragma once 
#ifndef SAFE_CAST
#include <sstream>
#include <stdexcept>
#include <limits>

#undef max

#define SAFE_CAST(t,f) safe_cast<t>(__FILE__, __LINE__, (f))

#define INT_CAST(x) safe_cast<int>(__FILE__, __LINE__, (x))
#define SHORT_CAST(x) safe_cast<short>(__FILE__, __LINE__, (x))

template <typename T, typename F>
T safe_cast(const char file[], size_t line, F input) {
	using std::numeric_limits;
	std::ostringstream msg;

	if( numeric_limits<F>::is_signed && !numeric_limits<T>::is_signed ) {
		if( input < 0 ) {
			msg << file << ":" << line << ": " 
				<< "signed value " << input << " cannot be cast to unsigned type";
			throw std::runtime_error(msg.str());
		}
	}
	if(  static_cast<size_t>(numeric_limits<T>::max()) < static_cast<size_t>(input) ) {
		msg << file << ":" << line << ": " 
			<< input << ", size " << sizeof(F) 
			<< ", cannot be cast to unsigned type of size" << sizeof(T);
		throw std::runtime_error(msg.str());
	}
	return static_cast<T>(input);
}
#endif
