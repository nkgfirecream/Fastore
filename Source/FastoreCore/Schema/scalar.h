#pragma once


#include "..\typedefs.h"
#include <sstream>

using namespace eastl;

// The physical representation of a scalar type
struct ScalarType
{
	size_t Size;
	int (*Compare)(void* left, void* right);
	fs::wstring (*ToString)(void* item);
	void (*Free)(void*);
};


//Free functions...
template <class T>
struct vpfree : public eastl::unary_function<void*, void>
{
	void operator()(void*& a)
	{
		//Do nothing.. It's a real type. We'll probably just overwrite the bytes
	}
};

//Indirect Delete for pointer types.
template <class T>
struct vpfree <T*>
{
	void operator()(void*& a)
	{
		delete *(void**)a;
	}
};


//Compare functions...
//Default compare types directly
template <class T>
struct vpcompare : public binary_function<void*, void*, int>
{
	int operator()(const void*& a, const void*& b) const
	{ 
		return *(T*)a > *(T*)b ? 1 : *(T*)a == *(T*)b ? 0 : -1;
	}
};

//Pointer types : deference and compare
template <class T>
struct vpcompare<T*>
{
	int operator()(const void*& a, const void*& b) const
	{ 
		return **(T**)a > **(T**)b ? 1 : **(T**)a == **(T**)b ? 0 : -1;
	}
};

template <>
struct vpcompare<long>
{
	int operator()(const void*& a, const void*& b) const
	{ 
		return *(long*)a - *(long*)b;
	}
};

template <>
struct vpcompare<long*>
{
	//TODO.. this probably isn't right.
	int operator()(const void*& a, const void*& b) const
	{ 
		return **(long**)a - **(long**)b;
	}
};

template <>
struct vpcompare<fs::wstring>
{

};

template <>
struct vpcompare<fs::wstring*>
{

};

template <>
struct vpcompare<int>
{
	int operator()(const void*& a, const void*& b) const
	{ 
		return *(int*)a - *(int*)b;
	}
};

template <>
struct vpcompare<int*>
{
	//TODO.. this probably isn't right.
	int operator()(const void*& a, const void*& b) const
	{ 
		return **(int**)a - **(int**)b;
	}
};

//String functions...
//Default try to cast to a string
template <class T>
struct vpstring : public unary_function<void*, fs::wstring>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		return (fs::wstring)a;
	}
};

//Pointers attempt to deference and then cast as string.
template <class T>
struct vpstring <T*>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		return (fs::wstring)*a;
	}
};

template <>
struct vpstring <fs::wstring>
{
	fs::wstring operator()(const void* a) const
	{
		return (fs::wstring)(wchar_t*)a;
	}
};

template <>
struct vpstring <fs::wstring*>
{
	fs::wstring operator()(const void* a) const
	{
		return *(fs::wstring*)a;
	}
};

template <>
struct vpstring <long>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		std::wstringstream mystream;
		mystream << (long)a;
		return mystream.str();
	}
};

template <>
struct vpstring <long*>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		std::wstringstream mystream;
		mystream << *(long*)a;
		return mystream.str();
	}
};

template <>
struct vpstring <int>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		std::wstringstream mystream;
		mystream << (int)a;
		return mystream.str();
	}
};

template <>
struct vpstring <int*>
{
	//TODO: smarter default template. This will fail in most cases.
	fs::wstring operator()(const void* a) const
	{
		std::wstringstream mystream;
		mystream << *(int*)a;
		return mystream.str();
	}
};

template <class T, class intCompare = vpcompare<T>, class stringFunction = vpstring<T>, class hashFunction = eastl::hash<T>, class freeFunction = vpfree<T>>
class Type
{
	public:
		intCompare Compare;
		stringFunction ToString;
		hashFunction Hash;
		freeFunction Free;

		size_t Size;
		const static size_t bucket_size = 4;
		const static size_t min_buckets = 8;

		size_t operator( )( const void* Key )const
		{
			return Hash((T)Key); 
		}

		bool operator( )(const void*& _Key1, const void*& _Key2) const
		{
			return Compare(_Key1, Key2) < 0;	
		}

		Type()
		{
			Size = sizeof(T);
		}
};