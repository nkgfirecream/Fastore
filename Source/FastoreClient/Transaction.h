#pragma once

#include "IDataAccess.h"
#include "Database.h"
#include "Dictionary.h"
#include "Encoder.h"
#include "Range.h"
#include "RangeSet.h"
#include "DataSet.h"
#include "Statistic.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
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
//using namespace System::Threading::Tasks;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			/// <Remarks>
			///   If a transaction is disposed and hasn't been committed or rolled back then it is automatically rolled back.
			///   Once a transaction has been committed or rolled back then it is in a new transaction state again and can be used as if it were new again.
			/// </Remarks>
			class Transaction : public IDataAccess, public IDisposable
			{
			private:
				class LogColumn
				{
				public:
					std::map<object*, object*> Includes;
					std::set<object*> Excludes;

				private:
					void InitializeInstanceFields();

public:
					LogColumn()
					{
						InitializeInstanceFields();
					}
				};

					private:
						boost::shared_ptr<Alphora::Fastore::Client::Database> privateDatabase;
					public:
						const boost::shared_ptr<Alphora::Fastore::Client::Database> &getDatabase() const;
						void setDatabase(const boost::shared_ptr<Alphora::Fastore::Client::Database> &value);
					private:
						bool privateReadIsolation;
					public:
						const bool &getReadIsolation() const;
						void setReadIsolation(const bool &value);
					private:
						bool privateWriteIsolation;
					public:
						const bool &getWriteIsolation() const;
						void setWriteIsolation(const bool &value);

			private:
				bool _completed;
				boost::shared_ptr<TransactionID> _transactionId;

				// Log entries - by column ID then by row ID - null value means exclude
				std::map<int, LogColumn*> _log;

			public:
				Transaction(const boost::shared_ptr<Alphora::Fastore::Client::Database> &database, bool readIsolation, bool writeIsolation);

				~Transaction();

				void Commit(bool flush = false);

			private:
				std::map<int, ColumnWrites*> GatherWrites();

			public:
				void Rollback();

				boost::shared_ptr<RangeSet> GetRange(int columnIds[], Range range, int limit, const boost::shared_ptr<object> &startId = nullptr);

				boost::shared_ptr<DataSet> GetValues(int columnIds[], object rowIds[]);

				void Include(int columnIds[], const boost::shared_ptr<object> &rowId, object row[]);

				void Exclude(int columnIds[], const boost::shared_ptr<object> &rowId);

				Statistic *GetStatistics(int columnIds[]);

			private:
				boost::shared_ptr<LogColumn> EnsureColumnLog(int columnId);

			public:
				std::map<int, TimeSpan> Ping();
			};
		}
	}
}
