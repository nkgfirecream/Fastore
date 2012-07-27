#include "Generator.h"

using namespace fastore::client;

Generator::Generator(const boost::shared_ptr<Database> &database, int podIDs[] = nullptr)
{
	InitializeInstanceFields();
	if (database == nullptr)
		throw boost::make_shared<ArgumentNullException>("database");
	_database = database;
	_podIDs = podIDs;
}

long long Generator::Generate(int columnId)
{
	// Take lock
	Monitor::Enter(_generatorLock);
	auto taken = true;
	try
	{
		// Find or create generator
		boost::shared_ptr<IDGenerator> generator;
		if (!_generators.TryGetValue(columnId, generator))
		{
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
			generator = boost::make_shared<IDGenerator>((size) => InternalGenerate(columnId, size));
			_generators.insert(make_pair(columnId, generator));
		}

		// Release lock
		Monitor::Exit(_generatorLock);
		taken = false;

		// Perform generation
		return generator->Generate();
	}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	finally
	{
		if (taken)
			Monitor::Exit(_generatorLock);
	}
}

long long Generator::InternalGenerate(int tableId, long long size)
{
	while (true)
	{
		try
		{
//C# TO C++ CONVERTER NOTE: The following 'using' block is replaced by its C++ equivalent:
//						using (var transaction = _database.Begin(true, true))
			auto transaction = _database->Begin(true, true);
			try
			{
				auto values = transaction->GetValues(std::map::GeneratorColumns, new object[] {tableId});
				Nullable<long long> result = Nullable<long long>();
				if (values->getCount() > 0)
				{
					result = static_cast<Nullable<long long>*>(values[0]->Values[0]);
					if (result.HasValue)
						transaction->Exclude(std::map::GeneratorColumns, tableId);
				}

				long long generatedValue = result.HasValue ? result.Value : 1;
				transaction->Include(std::map::GeneratorColumns, tableId, new object[] {generatedValue + size});

				transaction->Commit();
				return generatedValue;
			}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
			finally
			{
				if (transaction != 0)
					transaction.Dispose();
			}
		}
		catch (AggregateException *e)
		{
			if (IsNoWorkerForColumnException(dynamic_cast<ClientException*>(e->InnerException)))
				continue;
			else
				throw;
		}
		catch (ClientException *e)
		{
			if (IsNoWorkerForColumnException(e))
				continue;
			else
				throw;
		}
	}
}

bool Generator::IsNoWorkerForColumnException(const boost::shared_ptr<ClientException> &clientex)
{
	if (clientex != nullptr)
	{
		auto code = clientex->getData()["Code"];
		if (code != nullptr && static_cast<ClientException::Codes>(code) == ClientException::NoWorkerForColumn)
		{
			EnsureGeneratorTable();
			return true;
		}
	}
	return false;
}

void Generator::EnsureGeneratorTable()
{
	// Ensure that we have pod(s) to put the generator column into
	if (_podIDs == nullptr || sizeof(_podIDs) / sizeof(_podIDs[0]) == 0)
		DefaultPods();

	// Associate the generator column with each pod
//C# TO C++ CONVERTER NOTE: The following 'using' block is replaced by its C++ equivalent:
//				using (var transaction = _database.Begin(true, true))
	auto transaction = _database->Begin(true, true);
	try
	{
		// Add the generator column
		_database->Include(std::map::ColumnColumns, std::map::GeneratorNextValue, new object[] {std::map::GeneratorNextValue, "Generator.Generator", "Long", "Int", true});

		// Add the association with each pod
		for (int::const_iterator podID = _podIDs->begin(); podID != _podIDs->end(); ++podID)
			transaction->Include(std::map::PodColumnColumns, std::map::GeneratorNextValue, new object[] {podID, std::map::GeneratorNextValue});

		// Seed the column table to the first user ID
		transaction->Include(std::map::GeneratorColumns, new object[] {std::map::MaxClientColumnID}, new object[] {std::map::ColumnID});

		transaction->Commit();
	}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	finally
	{
		if (transaction != 0)
			transaction.Dispose();
	}
}

void Generator::DefaultPods()
{
	// Find a worker to put the generator table on
	Range podRange = Range();
	podRange.Ascending = true;
	podRange.ColumnID = std::map::PodID;
	auto podIds = _database->GetRange(new int[] {std::map::PodID}, podRange, 1);

	// Validate that there is at least one worker into which to place the generator
	if (podIds->getData()->getCount() == 0)
		throw boost::make_shared<ClientException>("Can't create generator column. No pods in hive.", ClientException::NoWorkersInHive);

	_podIDs = new int[] {static_cast<int>(podIds->getData()[0]->Values[0])};
}

void Generator::InitializeInstanceFields()
{
	_generatorLock = boost::make_shared<object>();
	_generators = std::map<int, IDGenerator*>();
}
