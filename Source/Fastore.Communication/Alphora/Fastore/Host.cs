/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;
using Thrift;
using Thrift.Collections;
using Thrift.Protocol;
using Thrift.Transport;
namespace Alphora.Fastore
{

  [Serializable]
  public partial class Host : TBase
  {
    private int _ID;
    private string _Address;

    public int ID
    {
      get
      {
        return _ID;
      }
      set
      {
        __isset.ID = true;
        this._ID = value;
      }
    }

    public string Address
    {
      get
      {
        return _Address;
      }
      set
      {
        __isset.Address = true;
        this._Address = value;
      }
    }


    public Isset __isset;
    [Serializable]
    public struct Isset {
      public bool ID;
      public bool Address;
    }

    public Host() {
    }

    public void Read (TProtocol iprot)
    {
      TField field;
      iprot.ReadStructBegin();
      while (true)
      {
        field = iprot.ReadFieldBegin();
        if (field.Type == TType.Stop) { 
          break;
        }
        switch (field.ID)
        {
          case 1:
            if (field.Type == TType.I32) {
              ID = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.String) {
              Address = iprot.ReadString();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          default: 
            TProtocolUtil.Skip(iprot, field.Type);
            break;
        }
        iprot.ReadFieldEnd();
      }
      iprot.ReadStructEnd();
    }

    public void Write(TProtocol oprot) {
      TStruct struc = new TStruct("Host");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (__isset.ID) {
        field.Name = "ID";
        field.Type = TType.I32;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(ID);
        oprot.WriteFieldEnd();
      }
      if (Address != null && __isset.Address) {
        field.Name = "Address";
        field.Type = TType.String;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteString(Address);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("Host(");
      sb.Append("ID: ");
      sb.Append(ID);
      sb.Append(",Address: ");
      sb.Append(Address);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
