#pragma once

#include "IDGenerator.h"
#include "Database.h"
#include "ClientException.h"
#include <map>
#include <boost/shared_ptr.hpp>


namespace fastore { namespace client
{
	class Generator
	{
	private:
		//TODO: Locking
		void* _generatorLock;
		std::map<int, IDGenerator> _generators;
		boost::shared_ptr<Database> _database;
//ORIGINAL LINE: private int[] _podIDs;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
		int *_podIDs;

		long long InternalGenerate(int tableId, long long size);

		bool IsNoWorkerForColumnException(const ClientException &clientex);

		void EnsureGeneratorTable();

		/// <summary> Defaults the pods based on available pods. </summary>
		void DefaultPods();

		void InitializeInstanceFields();

	public:
		Generator(const Database &database, std::vector<int> podIDs);

		/// <summary> Generates the next value for the given table. </summary>
		/// <param name="columnId"> The column an ID is being generated for. </param>
		long long Generate(int columnId);		
	};
}}
