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
			Data.Add("Code", (int)code);
		}
		protected ClientException(SerializationInfo info, StreamingContext context) : base(info, context) { }
		public ClientException(string message, Exception innerException) : base(message, innerException) { }

		public enum Codes
		{
			/// <summary> There is no worker for the present column. </summary>
			NoWorkerForColumn = 10000

			// TODO: fill out rest of codes and update throw sites
		}
	}
}
