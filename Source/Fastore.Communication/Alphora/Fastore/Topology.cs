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
  public partial class Topology : TBase
  {
    private int _topologyID;
    private Dictionary<int, Dictionary<int, List<int>>> _hosts;

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

    public Dictionary<int, Dictionary<int, List<int>>> Hosts
    {
      get
      {
        return _hosts;
      }
      set
      {
        __isset.hosts = true;
        this._hosts = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool topologyID;
      public bool hosts;
    }

    public Topology() {
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
                Hosts = new Dictionary<int, Dictionary<int, List<int>>>();
                TMap _map14 = iprot.ReadMapBegin();
                for( int _i15 = 0; _i15 < _map14.Count; ++_i15)
                {
                  int _key16;
                  Dictionary<int, List<int>> _val17;
                  _key16 = iprot.ReadI32();
                  {
                    _val17 = new Dictionary<int, List<int>>();
                    TMap _map18 = iprot.ReadMapBegin();
                    for( int _i19 = 0; _i19 < _map18.Count; ++_i19)
                    {
                      int _key20;
                      List<int> _val21;
                      _key20 = iprot.ReadI32();
                      {
                        _val21 = new List<int>();
                        TList _list22 = iprot.ReadListBegin();
                        for( int _i23 = 0; _i23 < _list22.Count; ++_i23)
                        {
                          int _elem24 = 0;
                          _elem24 = iprot.ReadI32();
                          _val21.Add(_elem24);
                        }
                        iprot.ReadListEnd();
                      }
                      _val17[_key20] = _val21;
                    }
                    iprot.ReadMapEnd();
                  }
                  Hosts[_key16] = _val17;
                }
                iprot.ReadMapEnd();
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
      TStruct struc = new TStruct("Topology");
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
      if (Hosts != null && __isset.hosts) {
        field.Name = "hosts";
        field.Type = TType.Map;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteMapBegin(new TMap(TType.I32, TType.Map, Hosts.Count));
          foreach (int _iter25 in Hosts.Keys)
          {
            oprot.WriteI32(_iter25);
            {
              oprot.WriteMapBegin(new TMap(TType.I32, TType.List, Hosts[_iter25].Count));
              foreach (int _iter26 in Hosts[_iter25].Keys)
              {
                oprot.WriteI32(_iter26);
                {
                  oprot.WriteListBegin(new TList(TType.I32, Hosts[_iter25][_iter26].Count));
                  foreach (int _iter27 in Hosts[_iter25][_iter26])
                  {
                    oprot.WriteI32(_iter27);
                  }
                  oprot.WriteListEnd();
                }
              }
              oprot.WriteMapEnd();
            }
          }
          oprot.WriteMapEnd();
        }
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("Topology(");
      sb.Append("TopologyID: ");
      sb.Append(TopologyID);
      sb.Append(",Hosts: ");
      sb.Append(Hosts);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
