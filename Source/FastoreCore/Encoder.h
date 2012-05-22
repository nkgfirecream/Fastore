#pragma once

#include "typedefs.h"

//TODO: Round two for the encoder...
//Shared encode buffer, works by reference to avoid needless copies.
class Encoder
{
	public:

		static void ReadString(const std::string&, std::string&);
		static void WriteString(const std::string&, std::string&);

		static void ReadBool(const std::string&, bool&);
		static void WriteBool(const bool&, std::string&);

		static void ReadInt(const std::string&, int&);
		static void WriteInt(const int&, std::string&);

		static void ReadLong(const std::string&, long long&);
		static void WriteLong(const long long&, std::string&);
};