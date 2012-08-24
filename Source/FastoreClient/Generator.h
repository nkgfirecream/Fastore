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
		boost::shared_ptr<Database> _database;

		std::vector<PodID>_podIDs;

		int InternalGenerate(int tableId, int size);

		bool IsNoWorkerForColumnException(const ClientException &clientex);

		void EnsureGeneratorTable();

		/// <summary> Defaults the pods based on available pods. </summary>
		void DefaultPods();

		void InitializeInstanceFields();

	public:
		Generator(boost::shared_ptr<Database> database, std::vector<PodID> podIDs);

		/// <summary> Generates the next value for the given table. </summary>
		/// <param name="columnId"> The column an ID is being generated for. </param>
		int Generate(int columnId);		
	};
}}
