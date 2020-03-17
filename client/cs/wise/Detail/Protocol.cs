using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;

namespace wise.Detail
{
    /// <summary>
    /// - 전송 패킹
    /// - 수신 시 패킷 만들기
    /// - 패킷 변형 처리 
    ///   - 암호화, 순서, 체크섬 (CRC32)
    /// </summary>
    internal class Protocol
    {
        internal static readonly int lengthFieldSize = 4;
        internal static readonly int topicFieldSize = 4;
        internal static readonly int headerSize = lengthFieldSize + topicFieldSize;

        private MemoryStream recvBuf; 
        private Packer packer;
        private byte[] dummyHeader;
        private Modifier sequenceModifier;
        private Modifier checksumModifier;
        private Modifier cipherModifier;

        public Protocol()
        {
            recvBuf = new MemoryStream();
            packer = new Packer();
            dummyHeader = new byte[headerSize];

            Buffer.BlockCopy(Encoding.UTF8.GetBytes("CAFEABBA"), 0, dummyHeader, 0, headerSize);

            sequenceModifier = new SequenceModifier();
            checksumModifier = new ChecksumModifier();
            cipherModifier = new CipherModifier(); 
        }

        /// <summary>
        /// Packetization. Modifier 처리
        /// </summary>
        public Result OnRecv(byte[] buf, int recvLen)
        {
            recvBuf.Write(buf, 0, recvLen);

            if ( recvBuf.Position < headerSize )
            {
                return Result.Success();
            }

            int recvBufLength = (int)recvBuf.Position;
            int currentPosition = 0;
            int totalProcessedLen = 0;

            while ( (recvBufLength - currentPosition) >= headerSize )
            {
                uint messageLength = GetMessageLength(recvBuf, currentPosition);

                if ( messageLength > Config.MaxPacketSize )
                {
                    return Result.Fail(Result.Code.FailTooLargeMessage); 
                }

                uint topic = GetMessageTopic(currentPosition);

                var packet = PacketFactory.Instance.Create(new Topic(topic));

                if ( packet == null )
                {
                    Network.Logger.Error(string.Format("Message not registered for {0}", topic));

                    return Result.Fail(Result.Code.FailMessageNotRegistered);
                }

                // 수신 시 패킷 변형
                var result = ModifyRecv(packet, recvBuf, currentPosition);

                if ( !result )
                {
                    return result;
                }

                // Unpack을 위해 메세지 시작 위치로 조정
                recvBuf.Position = currentPosition + headerSize;

                var rc = packet.Unpack(packer, recvBuf);

                if ( !rc )
                {
                    Network.Logger.Error(string.Format("Failed to Unpack for {0}", topic));
                    return Result.Fail(Result.Code.FailMessageUnpack);
                }

                int postCount = Network.Instance.Post(packet);

                if ( postCount == 0 )
                {
                    Network.Logger.Warn(string.Format("No Packet handler for Topic: {0}", packet.Topic));
                }

                totalProcessedLen += (int)messageLength;
                currentPosition += (int)messageLength;
            }

            // recvBuf.Position은 위 루프에서 메세지 별로 변경되었음.
            // 아래는 원래 길이인 recvBufLength만 참조해서 처리

            Debug.Assert(totalProcessedLen <= recvBufLength);

            byte[] arr = recvBuf.GetBuffer();

            Buffer.BlockCopy(arr, totalProcessedLen, buf, 0, (recvBufLength - totalProcessedLen));

            // 다음 번 Write 위치 맞춤
            recvBuf.Position = (recvBufLength - totalProcessedLen);

            return Result.Success();
        }

        /// <summary>
        /// Packetization. Modifier 처리
        /// 이 함수는 여러 쓰레드에서 호출 가능해야 함. 
        /// 아니면 블럭 되거나 쓰레드 안정성에 문제가 생김.
        /// </summary>
        public Result OnSend(Packet packet, MemoryStream sendBuf)
        {
            // sendBuf는 여러 메세지를 처리하기 위해 공유된다. 
            // 한번에 한 쓰레드에서만 접근한다. 
            // 버퍼 상의 위치를 가정할 수 없다. 

            int packetStartPosition = (int)sendBuf.Position;

            sendBuf.Write(dummyHeader, 0, 8);

            var rc = packet.Pack(packer, sendBuf);

            if ( !rc )
            {
                return Result.Fail(Result.Code.FailMessagePack);
            }

            // 현재 길이를 ModifySend에서 필요. 여기에 미리 지정하고 다시 수정하도록 함
            SetMessageLength(sendBuf, packetStartPosition, (uint)(sendBuf.Position - packetStartPosition));
            SetMessageTopic(sendBuf, packetStartPosition, packet.Topic);

            // 전송 시 패킷 변환
            var result = ModifySend(packet, sendBuf, packetStartPosition);

            if ( !result )
            {
                return result;
            }

            // 길이는 전체 버퍼 길이로 설정한다. 
            SetMessageLength(sendBuf, packetStartPosition, (uint)(sendBuf.Position - packetStartPosition));

            return Result.Success();
        }

        private Result ModifySend(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            if ( Config.EnableSequence && packet.IsSequenceEnabled )
            {
                var result = sequenceModifier.ModifySend(packet, packetBuf, packetStartPosition);

                if ( !result )
                {
                    return result;
                }
            }

            if ( Config.EnableChecksum && packet.IsChecksumEnabled )
            {
                var result = checksumModifier.ModifySend(packet, packetBuf, packetStartPosition);

                if ( !result )
                {
                    return result;
                }
            }

            if ( Config.EnableCipher && packet.IsCipherEnabled )
            {
                var result = cipherModifier.ModifySend(packet, packetBuf, packetStartPosition);

                if (!result)
                {
                    return result;
                }
            }

            return Result.Success();
        }

        private Result ModifyRecv(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            // 아직 packet의 내용이 차 있지는 않다. 먼저 검증 및 처리하고 Unpack한다. 

            if (Config.EnableCipher && packet.IsCipherEnabled)
            {
                var result = cipherModifier.ModifyRecv(packet, packetBuf, packetStartPosition);

                if (!result)
                {
                    return result;
                }
            }

            if (Config.EnableChecksum && packet.IsChecksumEnabled)
            {
                var result = checksumModifier.ModifyRecv(packet, packetBuf, packetStartPosition);

                if (!result)
                {
                    return result;
                }
            }

            if (Config.EnableSequence && packet.IsSequenceEnabled)
            {
                var result = sequenceModifier.ModifyRecv(packet, packetBuf, packetStartPosition);

                if (!result)
                {
                    return result;
                }
            }

            return Result.Success();
        }

        internal static uint GetMessageLength(MemoryStream stream, int currentPosition)
        {
            var buf = stream.GetBuffer();

            uint len = 0;

            int ri = currentPosition;

            len = buf[ri++];
            len |= (uint)(buf[ri++] << 8);
            len |= (uint)(buf[ri++] << 16);
            len |= (uint)(buf[ri++] << 24);

            return len;
        }

        private uint GetMessageTopic(int currentPosition)
        {
            var buf = recvBuf.GetBuffer();

            uint topic = 0;

            int ri = currentPosition + lengthFieldSize;

            topic = buf[ri++];
            topic |= (uint)(buf[ri++] << 8);
            topic |= (uint)(buf[ri++] << 16);
            topic |= (uint)(buf[ri++] << 24);

            return topic;
        }

        internal static void SetMessageLength(MemoryStream sendBuf, int packetStartPos, uint length)
        {
            var buf = sendBuf.GetBuffer();

            int ri = packetStartPos;

            buf[ri++] = (byte)(length & 0x000000FF);
            buf[ri++] = (byte)(length >> 8 & 0x000000FF);
            buf[ri++] = (byte)(length >> 16 & 0x000000FF);
            buf[ri++] = (byte)(length >> 24 & 0x000000FF);
        }

        private void SetMessageTopic(MemoryStream sendBuf, int packetStartPos, Topic topic)
        {
            var buf = sendBuf.GetBuffer();

            int ri = packetStartPos + lengthFieldSize;

            buf[ri++] = (byte)(topic.Key &       0x000000FF);
            buf[ri++] = (byte)(topic.Key >> 8 &  0x000000FF);
            buf[ri++] = (byte)(topic.Key >> 16 & 0x000000FF);
            buf[ri++] = (byte)(topic.Key >> 24 & 0x000000FF);
        }
    }
}
