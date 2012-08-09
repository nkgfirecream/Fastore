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
		boost::shared_ptr<boost::mutex> _lock;
		std::map<int, boost::shared_ptr<IDGenerator>> _generators;
		Database _database;

		std::vector<PodID>_podIDs;

		long long InternalGenerate(int tableId, long long size);

		bool IsNoWorkerForColumnException(const ClientException &clientex);

		void EnsureGeneratorTable();

		/// <summary> Defaults the pods based on available pods. </summary>
		void DefaultPods();

		void InitializeInstanceFields();

	public:
		Generator(const Database &database, std::vector<PodID> podIDs);

		/// <summary> Generates the next value for the given table. </summary>
		/// <param name="columnId"> The column an ID is being generated for. </param>
		long long Generate(int columnId);		
	};
}}
