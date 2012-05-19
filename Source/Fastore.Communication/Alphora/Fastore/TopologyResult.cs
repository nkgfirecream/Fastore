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
  public partial class TopologyResult : TBase
  {
    private Topology _topology;
    private long _revision;

    public Topology Topology
    {
      get
      {
        return _topology;
      }
      set
      {
        __isset.topology = true;
        this._topology = value;
      }
    }

    public long Revision
    {
      get
      {
        return _revision;
      }
      set
      {
        __isset.revision = true;
        this._revision = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool topology;
      public bool revision;
    }

    public TopologyResult() {
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
              Topology = new Topology();
              Topology.Read(iprot);
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.I64) {
              Revision = iprot.ReadI64();
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
      TStruct struc = new TStruct("TopologyResult");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (Topology != null && __isset.topology) {
        field.Name = "topology";
        field.Type = TType.Struct;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        Topology.Write(oprot);
        oprot.WriteFieldEnd();
      }
      if (__isset.revision) {
        field.Name = "revision";
        field.Type = TType.I64;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteI64(Revision);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("TopologyResult(");
      sb.Append("Topology: ");
      sb.Append(Topology== null ? "<null>" : Topology.ToString());
      sb.Append(",Revision: ");
      sb.Append(Revision);
      sb.Append(")");
      return sb.ToString();
    }

  }

}