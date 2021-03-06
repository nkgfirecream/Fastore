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
  public partial class OptionalHiveState : TBase
  {
    private HiveState _hiveState;
    private int _potentialWorkers;

    public HiveState HiveState
    {
      get
      {
        return _hiveState;
      }
      set
      {
        __isset.hiveState = true;
        this._hiveState = value;
      }
    }

    public int PotentialWorkers
    {
      get
      {
        return _potentialWorkers;
      }
      set
      {
        __isset.potentialWorkers = true;
        this._potentialWorkers = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool hiveState;
      public bool potentialWorkers;
    }

    public OptionalHiveState() {
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
            if (field.Type == TType.Struct) {
              HiveState = new HiveState();
              HiveState.Read(iprot);
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.I32) {
              PotentialWorkers = iprot.ReadI32();
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
      TStruct struc = new TStruct("OptionalHiveState");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (HiveState != null && __isset.hiveState) {
        field.Name = "hiveState";
        field.Type = TType.Struct;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        HiveState.Write(oprot);
        oprot.WriteFieldEnd();
      }
      if (__isset.potentialWorkers) {
        field.Name = "potentialWorkers";
        field.Type = TType.I32;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(PotentialWorkers);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("OptionalHiveState(");
      sb.Append("HiveState: ");
      sb.Append(HiveState== null ? "<null>" : HiveState.ToString());
      sb.Append(",PotentialWorkers: ");
      sb.Append(PotentialWorkers);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
