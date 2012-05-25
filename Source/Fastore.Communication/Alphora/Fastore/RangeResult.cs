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
  public partial class RangeResult : TBase
  {
    private List<ValueRows> _valueRowsList;
    private bool _EndOfRange;
    private bool _BeginOfRange;

    public List<ValueRows> ValueRowsList
    {
      get
      {
        return _valueRowsList;
      }
      set
      {
        __isset.valueRowsList = true;
        this._valueRowsList = value;
      }
    }

    public bool EndOfRange
    {
      get
      {
        return _EndOfRange;
      }
      set
      {
        __isset.EndOfRange = true;
        this._EndOfRange = value;
      }
    }

    public bool BeginOfRange
    {
      get
      {
        return _BeginOfRange;
      }
      set
      {
        __isset.BeginOfRange = true;
        this._BeginOfRange = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool valueRowsList;
      public bool EndOfRange;
      public bool BeginOfRange;
    }

    public RangeResult() {
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
                ValueRowsList = new List<ValueRows>();
                TList _list30 = iprot.ReadListBegin();
                for( int _i31 = 0; _i31 < _list30.Count; ++_i31)
                {
                  ValueRows _elem32 = new ValueRows();
                  _elem32 = new ValueRows();
                  _elem32.Read(iprot);
                  ValueRowsList.Add(_elem32);
                }
                iprot.ReadListEnd();
              }
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.Bool) {
              EndOfRange = iprot.ReadBool();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 3:
            if (field.Type == TType.Bool) {
              BeginOfRange = iprot.ReadBool();
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
      TStruct struc = new TStruct("RangeResult");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (ValueRowsList != null && __isset.valueRowsList) {
        field.Name = "valueRowsList";
        field.Type = TType.List;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.Struct, ValueRowsList.Count));
          foreach (ValueRows _iter33 in ValueRowsList)
          {
            _iter33.Write(oprot);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      if (__isset.EndOfRange) {
        field.Name = "EndOfRange";
        field.Type = TType.Bool;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteBool(EndOfRange);
        oprot.WriteFieldEnd();
      }
      if (__isset.BeginOfRange) {
        field.Name = "BeginOfRange";
        field.Type = TType.Bool;
        field.ID = 3;
        oprot.WriteFieldBegin(field);
        oprot.WriteBool(BeginOfRange);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("RangeResult(");
      sb.Append("ValueRowsList: ");
      sb.Append(ValueRowsList);
      sb.Append(",EndOfRange: ");
      sb.Append(EndOfRange);
      sb.Append(",BeginOfRange: ");
      sb.Append(BeginOfRange);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
