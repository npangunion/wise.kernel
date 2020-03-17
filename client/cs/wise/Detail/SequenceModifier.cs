using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace wise.Detail
{
    internal class SequenceModifier : Modifier
    {
        private byte recvSeq;
        private byte sendSeq;
        
        public SequenceModifier()
        {
            recvSeq = 1;
            sendSeq = 1;
        }

        public override Result ModifySend(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            packetBuf.WriteByte(sendSeq);

            Protocol.SetMessageLength(packetBuf, packetStartPosition, len + 1);
            
            IncSendSeq();

            return Result.Success();
        }

        public override Result ModifyRecv(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            var buf = packetBuf.GetBuffer();

            byte seq = buf[packetStartPosition + len - 1];

            if ( seq != recvSeq )
            {
                return Result.Fail(Result.Code.FailInvalidRecvSequence);
            }

            Protocol.SetMessageLength(packetBuf, packetStartPosition, len - 1);

            IncRecvSeq();

            return Result.Success();
        }

        private void IncSendSeq()
        {
            ++sendSeq;
        }

        private void IncRecvSeq()
        {
            ++recvSeq;
        }
    }
}
