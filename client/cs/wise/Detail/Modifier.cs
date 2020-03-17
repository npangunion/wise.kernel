using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace wise.Detail
{
    internal abstract class Modifier
    {
        public abstract Result ModifySend(Packet packet, MemoryStream packetBuf, int packetStartPosition);
        public abstract Result ModifyRecv(Packet packet, MemoryStream packetBuf, int packetStartPosition);
    }
}
