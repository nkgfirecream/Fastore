using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Alphora.Fastore.Client
{
	public class Generator
	{
		private Object _generatorLock;
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
					_generators.Add(tableId, new IDGenerator((size) => InternalGenerate(tableId, size)));
				
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
						long result = 1;
						if (values.Count > 0)
						{
							result = (long)values[0].Values[0];
							transaction.Exclude(GeneratorColumns, tableId);
						}
						transaction.Include(GeneratorColumns, tableId, new object[] { result + size });
						return result;
					}
					finally
					{
						//transaction.Commit();
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
			throw new NotImplementedException();
		}
	}
}
