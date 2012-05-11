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
  public partial class Query : TBase
  {
    private List<byte[]> _RowIDs;
    private List<RangeRequest> _Ranges;

    public List<byte[]> RowIDs
    {
      get
      {
        return _RowIDs;
      }
      set
      {
        __isset.RowIDs = true;
        this._RowIDs = value;
      }
    }

    public List<RangeRequest> Ranges
    {
      get
      {
        return _Ranges;
      }
      set
      {
        __isset.Ranges = true;
        this._Ranges = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool RowIDs;
      public bool Ranges;
    }

    public Query() {
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
                RowIDs = new List<byte[]>();
                TList _list30 = iprot.ReadListBegin();
                for( int _i31 = 0; _i31 < _list30.Count; ++_i31)
                {
                  byte[] _elem32 = null;
                  _elem32 = iprot.ReadBinary();
                  RowIDs.Add(_elem32);
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
                Ranges = new List<RangeRequest>();
                TList _list33 = iprot.ReadListBegin();
                for( int _i34 = 0; _i34 < _list33.Count; ++_i34)
                {
                  RangeRequest _elem35 = new RangeRequest();
                  _elem35 = new RangeRequest();
                  _elem35.Read(iprot);
                  Ranges.Add(_elem35);
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
      TStruct struc = new TStruct("Query");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (RowIDs != null && __isset.RowIDs) {
        field.Name = "RowIDs";
        field.Type = TType.List;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.String, RowIDs.Count));
          foreach (byte[] _iter36 in RowIDs)
          {
            oprot.WriteBinary(_iter36);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      if (Ranges != null && __isset.Ranges) {
        field.Name = "Ranges";
        field.Type = TType.List;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.Struct, Ranges.Count));
          foreach (RangeRequest _iter37 in Ranges)
          {
            _iter37.Write(oprot);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("Query(");
      sb.Append("RowIDs: ");
      sb.Append(RowIDs);
      sb.Append(",Ranges: ");
      sb.Append(Ranges);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
