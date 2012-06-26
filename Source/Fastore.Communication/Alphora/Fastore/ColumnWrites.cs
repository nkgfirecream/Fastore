/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
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
using System.Runtime.Serialization;
using Thrift.Protocol;
using Thrift.Transport;

namespace Alphora.Fastore
{

  #if !SILVERLIGHT
  [Serializable]
  #endif
  public partial class ColumnWrites : TBase
  {
    private List<Include> _includes;
    private List<Exclude> _excludes;

    public List<Include> Includes
    {
      get
      {
        return _includes;
      }
      set
      {
        __isset.includes = true;
        this._includes = value;
      }
    }

    public List<Exclude> Excludes
    {
      get
      {
        return _excludes;
      }
      set
      {
        __isset.excludes = true;
        this._excludes = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool includes;
      public bool excludes;
    }

    public ColumnWrites() {
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
            if (field.Type == TType.List) {
              {
                Includes = new List<Include>();
                TList _list28 = iprot.ReadListBegin();
                for( int _i29 = 0; _i29 < _list28.Count; ++_i29)
                {
                  Include _elem30 = new Include();
                  _elem30 = new Include();
                  _elem30.Read(iprot);
                  Includes.Add(_elem30);
                }
                iprot.ReadListEnd();
              }
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.List) {
              {
                Excludes = new List<Exclude>();
                TList _list31 = iprot.ReadListBegin();
                for( int _i32 = 0; _i32 < _list31.Count; ++_i32)
                {
                  Exclude _elem33 = new Exclude();
                  _elem33 = new Exclude();
                  _elem33.Read(iprot);
                  Excludes.Add(_elem33);
                }
                iprot.ReadListEnd();
              }
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
      TStruct struc = new TStruct("ColumnWrites");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (Includes != null && __isset.includes) {
        field.Name = "includes";
        field.Type = TType.List;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.Struct, Includes.Count));
          foreach (Include _iter34 in Includes)
          {
            _iter34.Write(oprot);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      if (Excludes != null && __isset.excludes) {
        field.Name = "excludes";
        field.Type = TType.List;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.Struct, Excludes.Count));
          foreach (Exclude _iter35 in Excludes)
          {
            _iter35.Write(oprot);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("ColumnWrites(");
      sb.Append("Includes: ");
      sb.Append(Includes);
      sb.Append(",Excludes: ");
      sb.Append(Excludes);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
