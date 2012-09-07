#define SAFE_CAST(t,f) safe_cast(__FILE__, __LINE__, (t), (f))

//Needed for Windows.
#ifdef max
#undef max
#endif

template <typename T, typename F>
T safe_cast(const char file[], size_t line, T, F input)
{
	using std::numeric_limits;
	std::ostringstream msg;

	if(numeric_limits<F>::is_signed && !numeric_limits<T>::is_signed)
	{
		if(input < 0)
		{
			msg << file << ":" << line << ": " 
			<< "signed value " << input << " cannot be cast to unsigned type";
			throw std::runtime_error(msg.str());
		}

		if(numeric_limits<T>::max() < static_cast<size_t>(input))
		{
			msg << file << ":" << line << ": " 
			<< input << ", size " << sizeof(F) 
			<< ", cannot be cast to unsigned type of size" << sizeof(T);
			throw std::runtime_error(msg.str());
		}
	}
	return static_cast<T>(input);
}