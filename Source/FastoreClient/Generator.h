#pragma once

#include "IDGenerator.h"
#include "Database.h"
#include "Dictionary.h"
#include "ClientException.h"
#include "Range.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Threading;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			class Generator
			{
			private:
				boost::shared_ptr<object> _generatorLock;
				std::map<int, IDGenerator*> _generators;
				boost::shared_ptr<Database> _database;
//ORIGINAL LINE: private int[] _podIDs;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
				int *_podIDs;

			public:
				Generator(const boost::shared_ptr<Database> &database, int podIDs[] = nullptr);

				/// <summary> Generates the next value for the given table. </summary>
				/// <param name="columnId"> The column an ID is being generated for. </param>
				long long Generate(int columnId);


			private:
				long long InternalGenerate(int tableId, long long size);

				bool IsNoWorkerForColumnException(const boost::shared_ptr<ClientException> &clientex);

				void EnsureGeneratorTable();

				/// <summary> Defaults the pods based on available pods. </summary>
				void DefaultPods();

			private:
				void InitializeInstanceFields();
			};
		}
	}
}
