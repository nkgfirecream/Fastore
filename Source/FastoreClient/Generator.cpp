﻿#include "Generator.h"
#include "Encoder.h"
#include "Dictionary.h"
#include "boost\assign\std\vector.hpp"
#include "boost\assign\list_of.hpp"

using namespace fastore::client;
using namespace boost::assign;

Generator::Generator(boost::shared_ptr<Database> database, std::vector<PodID> podIDs = std::vector<PodID>())
	: _database(database), _podIDs(podIDs)
{
	_lock = boost::shared_ptr<boost::mutex>(new boost::mutex());
}

int Generator::Generate(int columnId)
{
	// Take lock
	_lock->lock();
	bool taken = true;
	try
	{
		// Find or create generator
		auto generator = _generators.find(columnId);
		if (generator == _generators.end())
		{
			
			_generators.insert
			(
				std::pair<int, boost::shared_ptr<IDGenerator>>
				(
					columnId, 
					boost::shared_ptr<IDGenerator>
					(
						new IDGenerator
						(
							[&, columnId](int size) { return this->InternalGenerate(columnId, size); }
						)
					)
				)
			);

			generator = _generators.find(columnId);
		}

		// Release lock
		_lock->unlock();
		taken = false;

		// Perform generation
		return generator->second->Generate();
	}
	catch(std::exception& e)
	{
		if (taken)
			_lock->unlock();

		throw e;
	}
}

int Generator::InternalGenerate(int tableId, int size)
{
	while (true)
	{
		try
		{
			auto transaction = _database->Begin(true, true);
			try
			{
				std::string tableIdstring = Encoder<int>::Encode(tableId);
				auto values = transaction->GetValues(Dictionary::GeneratorColumns, list_of<std::string>(tableIdstring));
				
				int result = 0;

				if (values.size() > 0)
				{
						result = Encoder<int>::Decode(values[0].Values[0]);
						if (result > 0 /* TODO: NULL Marker! Right now we just get garbage in a result wasn't found */)
							transaction->Exclude(Dictionary::GeneratorColumns, tableIdstring);
						else
							result = Dictionary::MaxClientColumnID + 1; //Seed value.. don't start with zero because we may have added those manually.
				}

				transaction->Include(Dictionary::GeneratorColumns, tableIdstring, list_of<std::string>(Encoder<int>::Encode(result + size)));

				transaction->Commit();
				return result;
			}		
			catch (ClientException& e)
			{
				if (IsNoWorkerForColumnException(e))
					continue;
				else
					throw;
			}
		/*	catch (AggregateException *e)
			{
				if (IsNoWorkerForColumnException(dynamic_cast<ClientException*>(e->InnerException)))
					continue;
				else
					throw;
			}*/
		}
		catch(std::exception& e)
		{
			//Something happened while starting the transaction...
			throw e;
		}
	}
}

bool Generator::IsNoWorkerForColumnException(const ClientException &clientex)
{
	if (clientex.Code == ClientException::Codes::NoWorkerForColumn)
	{
		EnsureGeneratorTable();
		return true;
	}

	return false;
}

void Generator::EnsureGeneratorTable()
{
	// Ensure that we have pod(s) to put the generator column into
	if (_podIDs.empty())
		DefaultPods();

	auto transaction = _database->Begin(true, true);
	try
	{
		// Add the generator column
		transaction->Include
		(
			Dictionary::ColumnColumns,
			Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue), 
			list_of<std::string> 
				(Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue))
				("Generator.Generator")
				("Int")
				("Int")
				(Encoder<BufferType>::Encode(BufferType::Unique))
				(Encoder<bool>::Encode(true))
		);

		transaction->Commit();
		transaction = _database->Begin(true, true);

		// Add the association with each pod
		for (auto podID = _podIDs.begin(); podID != _podIDs.end(); ++podID)
		{
			transaction->Include
			(
				Dictionary::PodColumnColumns, 
				Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue), 
				list_of<std::string> 
					(Encoder<PodID>::Encode(*podID))
					(Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue))
			);
		}

		transaction->Commit();
		transaction = _database->Begin(true, true);

		// Seed the column table to the first user ID
		transaction->Include
		(
			Dictionary::GeneratorColumns, 
			Encoder<ColumnID>::Encode(Dictionary::ColumnID),
			list_of<std::string> 
				(Encoder<ColumnID>::Encode(Dictionary::MaxClientColumnID + 1))
		);

		transaction->Commit();
	}
	catch(std::exception e)
	{
		throw e;
	}
}

void Generator::DefaultPods()
{
	// Find a worker to put the generator table on
	Range podRange = Range();
	podRange.Ascending = true;
	podRange.ColumnID = Dictionary::PodID;
	auto podIds = _database->GetRange(list_of<ColumnID>(Dictionary::PodID), podRange, 1);

	// Validate that there is at least one worker into which to place the generator
	if (podIds.Data.size() == 0)
		throw ClientException("Can't create generator column. No pods in hive.", ClientException::Codes::NoWorkersInHive);

	_podIDs = list_of<PodID> (Encoder<PodID>::Decode(podIds.Data[0].Values[0]));
}
