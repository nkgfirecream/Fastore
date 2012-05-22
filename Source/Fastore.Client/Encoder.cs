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
            byte[] s = Encoding.UTF8.GetBytes(item);
            return s;
        }

        public static string ReadString(byte[] item)
        {
            return Encoding.UTF8.GetString(item); 
        }

        public static byte[] WriteBool(bool item)
        {
            return BitConverter.GetBytes(item);
        }

        public static bool ReadBool(byte[] item)
        {
            return BitConverter.ToBoolean(item, 0);
        }

        public static byte[] WriteLong(long item)
        {
            return BitConverter.GetBytes(item);
        }

        public static long ReadLong(byte[] item)
        {
            return BitConverter.ToInt64(item, 0);
        }

        public static byte[] WriteInt(int item)
        {
            return BitConverter.GetBytes(item);
        }

        public static int ReadInt(byte[] item)
        {
            return BitConverter.ToInt32(item, 0);
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
