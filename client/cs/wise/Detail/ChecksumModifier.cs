using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;

namespace wise.Detail
{
    internal class ChecksumModifier : Modifier
    {
        private HashAlgorithm crc; 

        public ChecksumModifier()
        {
            crc = Crc32.Create();
        }

        public override Result ModifySend(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            // 서버 구현에 맞춤. 비어 있으면 해시 없음
            if ( len == Protocol.headerSize )
            {
                return Result.Success();
            }

            var buf = packetBuf.GetBuffer();

            byte[] hash = crc.ComputeHash(
                buf, 
                packetStartPosition + Protocol.headerSize, 
                (int)(len - Protocol.headerSize) );

            packetBuf.Write(hash, 0, 4);

            Protocol.SetMessageLength(packetBuf, packetStartPosition, len + 4);

            return Result.Success();
        }

        public override Result ModifyRecv(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            // 서버 구현에 맞춤. 비어 있으면 해시 없음
            if ( len == Protocol.headerSize )
            {
                return Result.Success();
            }

            var buf = packetBuf.GetBuffer();

            byte[] hash = crc.ComputeHash(
                buf,
                packetStartPosition + Protocol.headerSize,
                (int)(len - Protocol.headerSize - 4));

            var crcIndex = packetStartPosition + (int)(len - 4);

            bool equal = hash[0] == buf[crcIndex + 0] &&
                         hash[1] == buf[crcIndex + 1] &&
                         hash[2] == buf[crcIndex + 2] &&
                         hash[3] == buf[crcIndex + 3];  

            if ( !equal)
            {
                return Result.Fail(Result.Code.FailInvalidCrc);
            }

            Protocol.SetMessageLength(packetBuf, packetStartPosition, len - 4);

            return Result.Success();
        }
    }
}
