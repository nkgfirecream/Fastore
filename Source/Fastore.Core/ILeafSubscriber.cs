using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface ILeafSubscriber<Key,Value>
    {
        void UpdateLink(Value row, int column, Leaf<Key, Value> leaf);
    }
}
