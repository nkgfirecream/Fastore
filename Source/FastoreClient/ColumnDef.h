#pragma once
#include <string>

namespace fastore { namespace client
{
	enum class BufferType
	{
		Identity = 0,
		Unique = 1,
		Multi = 2
	};

	class ColumnDef
	{
			private:
				int privateColumnID;
			public:
				const int &getColumnID() const;
				void setColumnID(const int &value);
			private:
				std::string privateName;
			public:
				const std::string &getName() const;
				void setName(const std::string &value);
			private:
				std::string privateType;
			public:
				const std::string &getType() const;
				void setType(const std::string &value);
			private:
				std::string privateIDType;
			public:
				const std::string &getIDType() const;
				void setIDType(const std::string &value);
			private:
				BufferType privateBufferType;
			public:
				const BufferType &getBufferType() const;
				void setBufferType(const BufferType &value);
	};
}}
