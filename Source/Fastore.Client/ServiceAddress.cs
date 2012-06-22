using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class ServiceAddress
    {
        public const int DefaultPort = 8765;

        public int Port { get; set; }
        public string Name { get; set; }

        public static ServiceAddress ParseOne(string address)
        {
            ServiceAddress result = new ServiceAddress();

            var components = address.Split(new[] { ':' }, StringSplitOptions.RemoveEmptyEntries);
            if (components.Length < 1 || components.Length > 2)
                throw new Exception("Must provide at least one address.");

            int port;
            if (components.Length < 2 || !Int32.TryParse(components[1], out port))
                port = DefaultPort;
            
            result.Port = port;
            result.Name = components[0].Trim();

			if (String.IsNullOrEmpty(result.Name))
				throw new Exception("Port is optional, but service host name must be given.");

			return result;
        }

		public static ServiceAddress[] ParseList(string composite)
        {
			var addresses = composite.Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
            if (addresses.Length < 1)
                throw new Exception("Must provide at least one address.");
            return (from a in addresses select ParseOne(a)).ToArray();
        }
	}
}
