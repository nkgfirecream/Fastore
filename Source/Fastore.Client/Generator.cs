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

		public Generator(Database database)
		{
			if (database == null)
				throw new ArgumentNullException("database");
			_database = database;
		}

		public long Generate(int tableId)
		{
			// Take lock
			Monitor.Enter(_generatorLock);
			var taken = true;
			try
			{
				// Find or create generator
				IDGenerator generator;
                if (!_generators.TryGetValue(tableId, out generator))
                {
                    generator = new IDGenerator((size) => InternalGenerate(tableId, size));
                    _generators.Add(tableId, generator);
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

		private static readonly int[] GeneratorColumns = new[] { ColumnDictionary.GeneratorNextValue };

		private long InternalGenerate(int tableId, long size)
		{
			while (true)
			{
                try
                {
                    // TODO: use transactions once they are working
                    //var transaction = _database.Begin(true, true);
                    var transaction = _database;
                    try
                    {
                        var values = transaction.GetValues(GeneratorColumns, new object[] { tableId });
                        long? result = null;
                        if (values.Count > 0)
                        {
                            result = (long?)values[0].Values[0];
                            if (result.HasValue)
                                transaction.Exclude(GeneratorColumns, tableId);
                        }

                        long generatedValue = result.HasValue ? result.Value : 1;
                        transaction.Include(GeneratorColumns, tableId, new object[] { generatedValue + size });
                        return generatedValue;
                    }
                    finally
                    {
                        //transaction.Commit();
                    }
                }
                catch (AggregateException e)
                {
                    var clientex = e.InnerException as ClientException;
                    if (clientex != null)
                    {
                        var code = clientex.Data["Code"];
                        if (code != null && (ClientException.Codes)code == ClientException.Codes.NoWorkerForColumn)
                        {
                            EnsureGeneratorTable();
                            continue;
                        }
                        throw;
                    }
                }
                catch (ClientException e)
                {
                    var code = e.Data["Code"];
                    if (code != null && (ClientException.Codes)code == ClientException.Codes.NoWorkerForColumn)
                    {
                        EnsureGeneratorTable();
                        continue;
                    }
                    throw;
                }
			}
		}

		private void EnsureGeneratorTable()
		{
            _database.Include
                (
                    new int[] { 0, 1, 2, 3, 4 }, 
                    ColumnDictionary.GeneratorNextValue, 
                    new object[] { ColumnDictionary.GeneratorNextValue, "Generator.Generator", "Long", "Int", true }
                ); 

			//Find a worker to put the generator table on
            Range podRange = new Range();
            podRange.Ascending = true;
            podRange.ColumnID = 300;

            var podIds = _database.GetRange(new int[] { 300 }, podRange, 1);

            if (podIds.Data.Count == 0)
                throw new ClientException("Can't create generator column. No Workers in hive.", ClientException.Codes.NoWorkersInHive);

            _database.Include(new int[] { 400, 401 }, ColumnDictionary.GeneratorNextValue, new object[] { podIds.Data[0].Values[0], ColumnDictionary.GeneratorNextValue });  
		}
	}
}
