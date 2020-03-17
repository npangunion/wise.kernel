using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{

    public class System
    {
        public enum Category
        {
            Sys,                        // system internal local messages
            Proto,                      // bits protocol messages if any  
            SysCategoryEnd = 16,      // end mark. reserved up to end
        }

        public enum Group
        {
            Net,
            Zen,
            SysGroupsEnd = 16
        }

        public enum Type
        {
            AcceptFailed,       // not used. 서버와 enum 맞추기위해 놔둠
            ConnectFailed,
            SessionFeady,
            SessionClosed,
            SysKeysEnd
        }
    }

    public class SysConnectFailed : Packet
    {
        private static Topic topic_ = new Topic(
            (byte)System.Category.Sys, (byte)System.Group.Net, (ushort)System.Type.ConnectFailed
            );

        public string Address;

        public static Topic GetTopic()
        {
            return topic_;
        }

        public SysConnectFailed()
            : base(GetTopic())
        {
        }
    }

    public class SysSessionReady : Packet
    {
        private static Topic topic_ = new Topic(
            (byte)System.Category.Sys, (byte)System.Group.Net, (ushort)System.Type.SessionFeady
            );

        public static Topic GetTopic()
        {
            return topic_;
        }

        public SysSessionReady()
            : base(GetTopic())
        {
        }
    }

    public class SysSessionClosed : Packet
    {
        private static Topic topic_ = new Topic(
            (byte)System.Category.Sys, (byte)System.Group.Net, (ushort)System.Type.SessionClosed
            );

        public static Topic GetTopic()
        {
            return topic_;
        }

        public SysSessionClosed()
            : base(GetTopic())
        {
        }
    }
} // wise 

