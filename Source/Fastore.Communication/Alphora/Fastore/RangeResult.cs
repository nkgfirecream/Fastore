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
    private bool _endOfFile;
    private bool _beginOfFile;
    private bool _limited;

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

    public bool EndOfFile
    {
      get
      {
        return _endOfFile;
      }
      set
      {
        __isset.endOfFile = true;
        this._endOfFile = value;
      }
    }

    public bool BeginOfFile
    {
      get
      {
        return _beginOfFile;
      }
      set
      {
        __isset.beginOfFile = true;
        this._beginOfFile = value;
      }
    }

    public bool Limited
    {
      get
      {
        return _limited;
      }
      set
      {
        __isset.limited = true;
        this._limited = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool valueRowsList;
      public bool endOfFile;
      public bool beginOfFile;
      public bool limited;
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
              EndOfFile = iprot.ReadBool();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 3:
            if (field.Type == TType.Bool) {
              BeginOfFile = iprot.ReadBool();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 4:
            if (field.Type == TType.Bool) {
              Limited = iprot.ReadBool();
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
      if (__isset.endOfFile) {
        field.Name = "endOfFile";
        field.Type = TType.Bool;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteBool(EndOfFile);
        oprot.WriteFieldEnd();
      }
      if (__isset.beginOfFile) {
        field.Name = "beginOfFile";
        field.Type = TType.Bool;
        field.ID = 3;
        oprot.WriteFieldBegin(field);
        oprot.WriteBool(BeginOfFile);
        oprot.WriteFieldEnd();
      }
      if (__isset.limited) {
        field.Name = "limited";
        field.Type = TType.Bool;
        field.ID = 4;
        oprot.WriteFieldBegin(field);
        oprot.WriteBool(Limited);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("RangeResult(");
      sb.Append("ValueRowsList: ");
      sb.Append(ValueRowsList);
      sb.Append(",EndOfFile: ");
      sb.Append(EndOfFile);
      sb.Append(",BeginOfFile: ");
      sb.Append(BeginOfFile);
      sb.Append(",Limited: ");
      sb.Append(Limited);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
