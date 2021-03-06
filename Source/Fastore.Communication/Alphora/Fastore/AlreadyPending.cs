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
  public partial class AlreadyPending : Exception, TBase
  {
    private TransactionID _pendingTransactionID;

    public TransactionID PendingTransactionID
    {
      get
      {
        return _pendingTransactionID;
      }
      set
      {
        __isset.pendingTransactionID = true;
        this._pendingTransactionID = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool pendingTransactionID;
    }

    public AlreadyPending() {
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
              PendingTransactionID = new TransactionID();
              PendingTransactionID.Read(iprot);
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
      TStruct struc = new TStruct("AlreadyPending");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (PendingTransactionID != null && __isset.pendingTransactionID) {
        field.Name = "pendingTransactionID";
        field.Type = TType.Struct;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        PendingTransactionID.Write(oprot);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("AlreadyPending(");
      sb.Append("PendingTransactionID: ");
      sb.Append(PendingTransactionID== null ? "<null>" : PendingTransactionID.ToString());
      sb.Append(")");
      return sb.ToString();
    }

  }

}
