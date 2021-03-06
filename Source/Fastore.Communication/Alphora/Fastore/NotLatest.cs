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
  public partial class NotLatest : Exception, TBase
  {
    private long _latest;

    public long Latest
    {
      get
      {
        return _latest;
      }
      set
      {
        __isset.latest = true;
        this._latest = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool latest;
    }

    public NotLatest() {
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
            if (field.Type == TType.I64) {
              Latest = iprot.ReadI64();
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
      TStruct struc = new TStruct("NotLatest");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (__isset.latest) {
        field.Name = "latest";
        field.Type = TType.I64;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteI64(Latest);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("NotLatest(");
      sb.Append("Latest: ");
      sb.Append(Latest);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
