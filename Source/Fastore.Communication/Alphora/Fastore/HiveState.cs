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
  public partial class HiveState : TBase
  {
    private int _topologyID;
    private Dictionary<int, ServiceState> _services;
    private int _hostID;

    public int TopologyID
    {
      get
      {
        return _topologyID;
      }
      set
      {
        __isset.topologyID = true;
        this._topologyID = value;
      }
    }

    public Dictionary<int, ServiceState> Services
    {
      get
      {
        return _services;
      }
      set
      {
        __isset.services = true;
        this._services = value;
      }
    }

    /// <summary>
    /// the host ID of the service giving the report
    /// </summary>
    public int HostID
    {
      get
      {
        return _hostID;
      }
      set
      {
        __isset.hostID = true;
        this._hostID = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool topologyID;
      public bool services;
      public bool hostID;
    }

    public HiveState() {
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
              TopologyID = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.Map) {
              {
                Services = new Dictionary<int, ServiceState>();
                TMap _map24 = iprot.ReadMapBegin();
                for( int _i25 = 0; _i25 < _map24.Count; ++_i25)
                {
                  int _key26;
                  ServiceState _val27;
                  _key26 = iprot.ReadI32();
                  _val27 = new ServiceState();
                  _val27.Read(iprot);
                  Services[_key26] = _val27;
                }
                iprot.ReadMapEnd();
              }
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 3:
            if (field.Type == TType.I32) {
              HostID = iprot.ReadI32();
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
      TStruct struc = new TStruct("HiveState");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (__isset.topologyID) {
        field.Name = "topologyID";
        field.Type = TType.I32;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(TopologyID);
        oprot.WriteFieldEnd();
      }
      if (Services != null && __isset.services) {
        field.Name = "services";
        field.Type = TType.Map;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteMapBegin(new TMap(TType.I32, TType.Struct, Services.Count));
          foreach (int _iter28 in Services.Keys)
          {
            oprot.WriteI32(_iter28);
            Services[_iter28].Write(oprot);
          }
          oprot.WriteMapEnd();
        }
        oprot.WriteFieldEnd();
      }
      if (__isset.hostID) {
        field.Name = "hostID";
        field.Type = TType.I32;
        field.ID = 3;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(HostID);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("HiveState(");
      sb.Append("TopologyID: ");
      sb.Append(TopologyID);
      sb.Append(",Services: ");
      sb.Append(Services);
      sb.Append(",HostID: ");
      sb.Append(HostID);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
