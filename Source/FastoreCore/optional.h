#pragma once

template<typename T> 
class Optional
{
	public:	
		Optional()
		{
			_hasValue = false;		
		}

		Optional(const T t)
		{		
			_hasValue = true;
			_value = t;
		}

		Optional(const Optional<T>& other)
		{
			_hasValue = other._hasValue;
			if (_hasValue)
				_value = other._value;
		}

		// Overloads

		operator T()
		{
			if (_hasValue)
				return _value;
			else
				throw new exception("Object does not have a value.");
		}
	
		void operator =(void* p)
		{
			if (p != NULL)
				throw new InvalidOperationException ("Optional value must be given null pointer.");
			_hasValue = true;		
		}

		const T operator =(const T& t)
		{
			_hasValue = true;
			_value = t
			return _value;
		}	

		T operator *()
		{
			return _value;
		}

		bool operator ==(void* p)
		{
			return p == NULL && !_hasValue;
		}

		bool operator !=(void* p)
		{
			return !operator ==(p);
		}	

	private:
		bool _hasValue;
		T _value;
};

