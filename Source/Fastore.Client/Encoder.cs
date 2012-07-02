using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    static class Encoder
    {
        public static byte[] WriteString(string item)
        {
            return Encoding.UTF8.GetBytes(item);
        }

        public static string ReadString(byte[] item)
        {
            if (item.Length > 0)
                return Encoding.UTF8.GetString(item);
            else
                return null;
        }

        public static byte[] WriteBool(bool item)
        {
            return BitConverter.GetBytes(item);
        }

        public static bool? ReadBool(byte[] item)
        {
            if (item.Length > 0)
                return BitConverter.ToBoolean(item, 0);
            else
                return null;
        }

        public static byte[] WriteLong(long item)
        {
            return BitConverter.GetBytes(item);
        }

        public static long? ReadLong(byte[] item)
        {
            if (item.Length > 0)
                return BitConverter.ToInt64(item, 0);
            else
                return null;
        }

        public static byte[] WriteInt(int item)
        {
            return BitConverter.GetBytes(item);
        }

        public static int? ReadInt(byte[] item)
        {
            if (item.Length > 0)
                return BitConverter.ToInt32(item, 0);
            else
                return null;
        }

        public static byte[] Encode(object item)
        {
            var type = item.GetType();
            if (type == typeof(int))
                return WriteInt((int)item);
            if (type == typeof(string))
                return WriteString((string)item);
            if (type == typeof(long))
                return WriteLong((long)item);
            if (type == typeof(bool))
                return WriteBool((bool)item);

            throw new Exception("Unsupported Type");
        }

        public static object Decode(byte[] item, string type)
        {
            object toReturn = null;
            switch (type)
            {
                case "Int":
                    toReturn = ReadInt(item);
                    break;
                case "String":
                    toReturn = ReadString(item);
                    break;
                case "Long":
                    toReturn = ReadLong(item);
                    break;
                case "Bool":
                    toReturn = ReadBool(item);
                    break;
                default:
                    throw new Exception("Unsupported Type");
            }

            return toReturn;
        }
    }
}
