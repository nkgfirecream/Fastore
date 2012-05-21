#pragma once

#include "typedefs.h"

//TODO: Round two for the encoder...
//Shared encode buffer, works by reference to avoid needless copies.
class Encoder
{
	public:
		static Encoder* GetEncoder();

		void ReadString(const std::string&, std::string&);
		void WriteString(const std::string&, std::string&);

		void ReadBool(const std::string&, bool&);
		void WriteBool(const bool&, std::string&);

		void ReadInt(const std::string&, int&);
		void WriteInt(const int&, std::string&);

		void ReadLong(const std::string&, long long&);
		void WriteLong(const long long&, std::string&);

	private:
		static Encoder* _encoder;
		char* _buffer;
		
		Encoder();
};