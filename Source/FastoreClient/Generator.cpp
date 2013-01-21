#include "Generator.h"
#include "Encoder.h"
#include "Dictionary.h"
#include <Schema/Dictionary.h>
#include "boost/assign/std/vector.hpp"
#include "boost/assign/list_of.hpp"

#include "../FastoreCommon/Log/Syslog.h"

using namespace fastore::client;
using namespace boost::assign;

using fastore::Syslog;
using fastore::log_err;
using fastore::log_endl;

Generator::Generator(boost::shared_ptr<Database> database, std::vector<PodID> podIDs = std::vector<PodID>())
	: _database(database), _podIDs(podIDs)
{
	_lock = boost::shared_ptr<boost::mutex>(new boost::mutex());
	EnsureGeneratorTable();
}

int64_t Generator::Generate(int64_t generatorId, boost::optional<int64_t> minId)
{
	// Take lock
	_lock->lock();
	bool taken = true;
	try
	{
		// Find or create generator
		auto generator = _generators.find(generatorId);
		if (generator == _generators.end())
		{
			
			_generators.insert
			(
				std::pair<int64_t, boost::shared_ptr<IDGenerator>>
				(
					generatorId, 
					boost::shared_ptr<IDGenerator>
					(
						new IDGenerator
						(
							[&, generatorId, minId](int size) { return this->InternalGenerate(generatorId, size, minId); }
						)
					)
				)
			);

			generator = _generators.find(generatorId);
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

int64_t Generator::InternalGenerate(int64_t generatorId, int size, boost::optional<int64_t> minId)
{
	while (true)
	{
		try
		{
			auto transaction = _database->begin(true);
			try
			{
				std::string generatorIdstring = Encoder<int64_t>::Encode(generatorId);
				auto values = transaction->getValues(Dictionary::GeneratorColumns, list_of<std::string>(generatorIdstring));
				
				int64_t result = minId ? *minId : 0;

				//If we have an entry increment, otherwise leave at default.
				if (values.size() > 0 && values[0].Values[0].__isset.value )
				{	
					transaction->exclude(Dictionary::GeneratorColumns, generatorIdstring);
					int64_t tempresult = Encoder<int64_t>::Decode(values[0].Values[0].value);
					if (result < tempresult)
						result = tempresult;
				}

				transaction->include(Dictionary::GeneratorColumns, generatorIdstring, list_of<std::string>(Encoder<int64_t>::Encode(result + size)));

				transaction->commit();
				return result;
			}		
			catch (ClientException& e)
			{
				Log << log_err << __func__ << ": " << e.what() << log_endl;
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
		// no point catching just to rethrow
		catch(std::exception&) 
		{
			//Something happened while starting the transaction...
			throw;
		}
	}
}

void Generator::EnsureGeneratorTable()
{
	fastore::client::RangeBound bound;
	bound.Inclusive = true;
	bound.Bound = Encoder<ColumnID>::Encode(fastore::client::Dictionary::GeneratorNextValue);

	fastore::client::Range range;
	range.Ascending = true;
	range.ColumnID = fastore::common::Dictionary::ColumnID;
	range.Start = bound;
	range.End = bound;	

	auto result = _database->getRange(fastore::common::Dictionary::ColumnColumns, range, 1);

	if (result.Data.empty())
	{
		// Ensure that we have pod(s) to put the generator column into
		if (_podIDs.empty())
			DefaultPods();

		auto transaction = _database->begin(true);
		// Add the generator column
		transaction->include
		(
			fastore::common::Dictionary::ColumnColumns,
			Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue), 
			list_of<std::string> 
				(Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue))
				("Generator.Generator")
				("Long")
				("Long")
				(Encoder<BufferType_t>::Encode(BufferType_t::Multi))
				(Encoder<bool>::Encode(true))
		);

		// Add the association with each pod
		for (auto podID = _podIDs.begin(); podID != _podIDs.end(); ++podID)
		{
			transaction->include
			(
				fastore::common::Dictionary::PodColumnColumns, 
				Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue), 
				list_of<std::string> 
					(Encoder<PodID>::Encode(*podID))
					(Encoder<ColumnID>::Encode(Dictionary::GeneratorNextValue))
			);
		}

		// Seed the column table to the first user ID
		transaction->include
		(
			Dictionary::GeneratorColumns, 
			Encoder<ColumnID>::Encode(fastore::common::Dictionary::ColumnID),
			list_of<std::string> 
				(Encoder<ColumnID>::Encode(fastore::common::Dictionary::MaxClientColumnID + 1))
		);

		transaction->commit();
	}
}

void Generator::DefaultPods()
{
	// Find a worker to put the generator table on
	Range podRange = Range();
	podRange.Ascending = true;
	podRange.ColumnID = fastore::common::Dictionary::PodID;
	RangeSet podIds = _database->getRange(list_of<ColumnID>(fastore::common::Dictionary::PodID), podRange, 1);

	// Validate that there is at least one worker into which to place the generator
	if (podIds.Data.size() == 0 || !podIds.Data[0].Values[0].__isset.value)
		throw ClientException("Can't create generator column. No pods in hive.", ClientException::Codes::NoWorkersInHive);

	_podIDs.resize(1, Encoder<PodID>::Decode(podIds.Data[0].Values[0].value));
}
