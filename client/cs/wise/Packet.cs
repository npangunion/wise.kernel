using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace wise
{
    public class Packet : Message
    {
        public Packet(Topic topic)
            : base(topic)
        {
        }

        public bool HasSession
        {
            get { return Session != null; }
        }

        public Session Session
        {
            get; set;
        }

        public bool IsSequenceEnabled
        {
            get;
            protected set;
        }

        public bool IsChecksumEnabled
        {
            get;
            protected set;
        }

        public bool IsCipherEnabled
        {
            get;
            protected set;
        }

        public virtual bool Pack(Packer packer, Stream stream)
        {
            return true;
        }

        public virtual bool Unpack(Packer packer, Stream stream)
        {
            return true;
        }

        public override string ToString()
        {
            return string.Format("{0} Session: {1}", base.ToString(), Session.Id);
        }
    }
}
