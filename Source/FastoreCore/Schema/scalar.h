#pragma once
#include <string>

#include "../safe_cast.h"

// The physical representation of a scalar type
struct ScalarType
{
	typedef int (*CompareFunc)(const void* left, const void* right);
	typedef int (*IndexOfFunc)(const char* items, const int count, void *key);
	typedef std::wstring (*ToStringFunc)(const void* item);
	typedef void (*CopyInFunc)(const void* source, void* arraypointer);
	typedef void (*CopyOutFunc)(const void* arraypointer, std::string& destination); 
	typedef void (*DeallocateFunc)(void* items, const int count);
	typedef void* (*GetPointerFunc)(const std::string& source);

	size_t Size;
	ToStringFunc ToString;
	CompareFunc Compare;
	IndexOfFunc IndexOf;
	CopyInFunc CopyIn;
	CopyOutFunc CopyOut;
	std::string Name;
	DeallocateFunc Deallocate;
	GetPointerFunc GetPointer;
	
	bool operator ()(const void* left, const void* right) const
	{
		return Compare(left, right) <= 0;
	}
};

//Helper Functions

void NoOpDeallocate(void* items, const int count);

template <typename T>
int ScaledIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;
	T loVal;
	T hiVal;
	T val;

	while (lo <= hi)
	{
		loVal = ((T*)items)[lo];
		hiVal = ((T*)items)[hi];
		val = *(T*)key;

		// Test for value on or over range bounds
		if (val <= loVal)
			return val == loVal ? lo : ~lo;
		if (val >= hiVal)
			return val == hiVal ? hi : ~(hi + 1);

		// Split proportionately to the value scaling
		split = lo + (int)((val - (double)loVal) / (hiVal - (double)loVal) * (hi - lo));
			//(val - loVal) * (hi - lo) / (hiVal - loVal) + lo;		integer version suffers from overflow problems with subtraction and multiplication

		result = val - ((T*)items)[split];

		if (result == 0)
			return split;
		else if (result < 0)
		{
			hi = split - 1;
			++lo;
		}
		else
		{
			lo = split + 1;
			--hi;
		}
	}

	return ~lo;
}

template <typename T>
inline int SignNum(T value)
{
	return (value > 0) - (value < 0);
}

template <typename T>
bool ReasonablySmall( T value )
{
  if( ! std::numeric_limits<T>::is_integer ) 
    return true;

  if( std::numeric_limits<T>::is_signed && value < 0 ) 
    value = -value;

  enum { ndigits = std::numeric_limits<T>::digits };
  static const T max_root = std::numeric_limits<T>::max() >> (ndigits/2);

  return max_root < value;
}

template <typename T>
int TargetedIndexOf(const char* item_buf, const int count, void *key)
{
	if (count == 0)
		return ~0;

	int hi = count - 1;
	const T* items =  reinterpret_cast<const T*>(item_buf);
	T val = *reinterpret_cast<T*>(key);

	T loVal = items[0];
	T hiVal = items[hi];

	// Test for value on or over range bounds
	if (val <= loVal)
		return val == loVal ? 0 : ~0;
	if (val >= hiVal)
		return val == hiVal ? hi : ~count;

	// Split proportionately to the value scaling
	T pos = ReasonablySmall(hi)? 
	           hi * (val - loVal) / (hiVal - loVal)
	         :
	                (val - loVal) / ((hiVal - loVal) / hi);

	T diff = val - items[pos];

	if (diff == 0)
	  return INT_CAST(pos);

	//normalize direction
	int direction = SignNum(diff);

	if (direction > 0)
	{
		while (diff > 0)
		{
			if (++pos > hi)
				return ~count;
			diff = val - items[pos];
		}
	}
	else
	{
		while (diff < 0)
		{
			--pos;
			if (pos < 0)
				return ~0;
			diff = val - items[pos];
		}
	}
	return INT_CAST(diff == 0 ? pos : ~pos);
}

template <typename T>
int NumericIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi)  >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		result = *(T*)key - ((T*)items)[split];

		if (result == 0)
			return split;
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	return ~lo;
}

template <typename T, ScalarType::CompareFunc Comparer>
int CompareIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi) >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		result = Comparer((T*)key, &((T*)items)[split]);

		if (result == 0)
			return split;
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	return ~lo;
}

template <typename T>
void CopyToArray(const void* item, void* arrpointer)
{
	new (arrpointer) T(*(T*)item);
}

template <typename T>
void* GetPointerFromString(const std::string& source)
{
	return (void*)(source.data());
}

template <typename T>
void CopyOutOfArray(const void* arrpointer, std::string& destination)
{
	destination.assign((const char*)arrpointer, sizeof(T));
}
