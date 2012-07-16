using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace Alphora.Fastore.Client
{
	public class ClientException : Exception
	{
		public ClientException() { }
		public ClientException(string message) : base(message) { }
		public ClientException(string message, Codes code) : base(message) 
		{
			Data.Add("Code", code);
		}
		protected ClientException(SerializationInfo info, StreamingContext context) : base(info, context) { }
		public ClientException(string message, Exception innerException) : base(message, innerException) { }

		public enum Codes
		{
			/// <summary> There is no worker for the present column. </summary>
			NoWorkerForColumn = 10000,
            NoWorkersInHive = 10001

			// TODO: fill out rest of codes and update throw sites
		}

		public static void ThrowErrors(List<Exception> errors)
		{
			if (errors.Count > 0)
			{
				if (errors.Count == 1)
					throw errors[0];
				else
					throw new AggregateException(errors);
			}
		}

		public static void ForceCleanup(params Action[] actions)
		{
			var errors = new List<Exception>();
			foreach (var action in actions)
				try
				{
					action();
				}
				catch (Exception e)
				{
					errors.Add(e);
				}
			ThrowErrors(errors);
		}
	}
}
