using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Alphora.Fastore.Client
{
	public class Generator
	{
		private Object _generatorLock = new object();
		private Dictionary<int, IDGenerator> _generators = new Dictionary<int, IDGenerator>();
		private Database _database;
		private int[] _podIDs;

		public Generator(Database database, int[] podIDs = null)
		{
			if (database == null)
				throw new ArgumentNullException("database");
			_database = database;
			_podIDs = podIDs;
		}

		/// <summary> Generates the next value for the given table. </summary>
		/// <param name="columnId"> The column an ID is being generated for. </param>
		public long Generate(int columnId)
		{
			// Take lock
			Monitor.Enter(_generatorLock);
			var taken = true;
			try
			{
				// Find or create generator
				IDGenerator generator;
                if (!_generators.TryGetValue(columnId, out generator))
                {
                    generator = new IDGenerator((size) => InternalGenerate(columnId, size));
                    _generators.Add(columnId, generator);
                }
				
				// Release lock
				Monitor.Exit(_generatorLock);
				taken = false;

				// Perform generation
				return generator.Generate();
			}
			finally
			{
				if (taken)
					Monitor.Exit(_generatorLock);
			}
		}


		private long InternalGenerate(int tableId, long size)
		{
			while (true)
			{
                try
                {
                    using (var transaction = _database.Begin(true, true))
                    {
                        var values = transaction.GetValues(Dictionary.GeneratorColumns, new object[] { tableId });
                        long? result = null;
                        if (values.Count > 0)
                        {
                            result = (long?)values[0].Values[0];
                            if (result.HasValue)
								transaction.Exclude(Dictionary.GeneratorColumns, tableId);
                        }

                        long generatedValue = result.HasValue ? result.Value : 1;
						transaction.Include(Dictionary.GeneratorColumns, tableId, new object[] { generatedValue + size });

						transaction.Commit();
                        return generatedValue;
                    }
                }
                catch (AggregateException e)
                {
                    if (IsNoWorkerForColumnException(e.InnerException as ClientException))
						continue;
					else
                        throw;
                }
                catch (ClientException e)
                {
					if (IsNoWorkerForColumnException(e))
						continue;
					else
						throw;
                }
			}
		}

		private bool IsNoWorkerForColumnException(ClientException clientex)
		{
            if (clientex != null)
            {
                var code = clientex.Data["Code"];
                if (code != null && (ClientException.Codes)code == ClientException.Codes.NoWorkerForColumn)
                {
                    EnsureGeneratorTable();
                    return true;
                }
            }
			return false;
		}

		private void EnsureGeneratorTable()
		{
			// Ensure that we have pod(s) to put the generator column into
			if (_podIDs == null || _podIDs.Length == 0)
				DefaultPods();

			// Associate the generator column with each pod
			using (var transaction = _database.Begin(true, true))
			{
				// Add the generator column
				_database.Include
				(
					Dictionary.ColumnColumns,
					Dictionary.GeneratorNextValue,
					new object[] { Dictionary.GeneratorNextValue, "Generator.Generator", "Long", "Int", true }
				);
				
				// Add the association with each pod
				foreach (var podID in _podIDs)
					transaction.Include(Dictionary.PodColumnColumns, Dictionary.GeneratorNextValue, new object[] { podID , Dictionary.GeneratorNextValue });  
				
				// Seed the column table to the first user ID
				transaction.Include(Dictionary.GeneratorColumns, new object[] { Dictionary.MaxClientColumnID }, new object[] { Dictionary.ColumnID });

				transaction.Commit();
			}
		}

		/// <summary> Defaults the pods based on available pods. </summary>
		private void DefaultPods()
		{
			// Find a worker to put the generator table on
			Range podRange = new Range();
			podRange.Ascending = true;
			podRange.ColumnID = Dictionary.PodID;
			var podIds = _database.GetRange(new int[] { Dictionary.PodID }, podRange, 1);

			// Validate that there is at least one worker into which to place the generator
			if (podIds.Data.Count == 0)
				throw new ClientException("Can't create generator column. No pods in hive.", ClientException.Codes.NoWorkersInHive);

			_podIDs = new[] { (int)podIds.Data[0].Values[0] };
		}
	}
}
