using System;
using System.IO;
using System.Text;
using System.Security.Cryptography;

namespace wise.Detail
{
    internal class CipherModifier : Modifier
    {
        private class Cipher
        {
            public enum Direction
            {
                ENCRYPTION, 
                DECRYPTION
            } 

            AesCryptoServiceProvider crypto;
            int count;
            Direction dir;
            SHA1 hasher;
            MemoryStream hashStream;
            MemoryStream workStream;

            public byte[] Key
            {
                get; set;
            }

            public byte[] IV
            {
                get; set;
            }

            public Cipher(Direction dir)
            {
                this.dir = dir;
                hasher = new SHA1CryptoServiceProvider();
                hashStream = new MemoryStream();
                workStream = new MemoryStream();
            }

            public void Start()
            {
                crypto = new AesCryptoServiceProvider();
                crypto.BlockSize = 128;
                crypto.KeySize = 128;
                crypto.Key = Key;
                crypto.IV = IV;
                crypto.Padding = PaddingMode.PKCS7;
                crypto.Mode = CipherMode.CBC;
            }

            public void UpdateHash(byte[] data, int offset, int length)
            {
                // 64 바이트 까지만 반영한다. 
                int len = Math.Min(length, 64);

                // SHA1은 20바이트 hash 생성. 

                workStream.Position = 0;
                workStream.Write(data, offset, len);
                workStream.Write(hashStream.GetBuffer(), 0, (int)hashStream.Length);

                var hashValue = hasher.ComputeHash(workStream.GetBuffer());

                hashStream.Position = 0;
                hashStream.Write(hashValue, 0, hashValue.Length);
            }

            public byte[] TransformFinalBlock(byte[] inputBuffer, int inputOffset, int inputCount)
            {
                if (dir == Direction.ENCRYPTION)
                {
                    using (ICryptoTransform transform = crypto.CreateEncryptor(crypto.Key, IV))
                    {
                        return transform.TransformFinalBlock(inputBuffer, inputOffset, inputCount);
                    } 
                }
                else
                {
                    using (ICryptoTransform transform = crypto.CreateDecryptor(crypto.Key, IV))
                    {
                        return transform.TransformFinalBlock(inputBuffer, inputOffset, inputCount);
                    }
                }
            }

            public void Complete()
            {
                if (count % Config.CipherKeyChangeInterval == 0)
                {
                    UpdateKey();
                }

                UpdateIV();

                ++count;
            } 

            private void UpdateIV()
            {
                byte[] tv = new byte[16];
                Array.Copy(hashStream.GetBuffer(), 0, tv, 0, 16);

                IV = tv;
            }

            private void UpdateKey()
            {
                byte[] tv = new byte[16];
                Array.Copy(hashStream.GetBuffer(), 0, tv, 0, 16);

                Key = tv;

                Start();
            }
        }

        Cipher sender;
        Cipher recevier;

        public CipherModifier()
        {
            sender = new Cipher(Cipher.Direction.ENCRYPTION);
            recevier = new Cipher(Cipher.Direction.DECRYPTION);

            sender.Key = HexStringToBytes("2B7E151628AED2A6ABF7158809CF4F3C");
            sender.IV = HexStringToBytes("ACE03D519F3CBA2BF67CF1B7E1C4F02D"); 

            recevier.Key = HexStringToBytes("2B7E151628AED2A6ABF7158809CF4F3C");
            recevier.IV = HexStringToBytes("ACE03D519F3CBA2BF67CF1B7E1C4F02D");

            sender.Start();
            recevier.Start();
        }
 
        public override Result ModifySend(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            int BLOCK_SIZE = 16;

            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            // 스트림에 패드 크기를 확보해 둔다. 아래 구현은 서버와 같다.  
           var padSize = BLOCK_SIZE - (len - Protocol.headerSize) % BLOCK_SIZE;
            var pad = HexStringToBytes("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
            packetBuf.Write(pad, 0, (int)padSize);

            byte[] buf = packetBuf.GetBuffer();

            // 해시 데이터 반영
            sender.UpdateHash(buf, packetStartPosition + Protocol.headerSize, (int)(len - Protocol.headerSize));

            byte[] encryptedData = sender.TransformFinalBlock( buf, packetStartPosition + Protocol.headerSize, (int)(len - Protocol.headerSize));
            Buffer.BlockCopy(encryptedData, 0, buf, packetStartPosition + Protocol.headerSize, encryptedData.Length);

            // 완료 처리
            sender.Complete();

            // 패킷 길이 변경
            Protocol.SetMessageLength(packetBuf, packetStartPosition, (uint)(encryptedData.Length + Protocol.headerSize));

            return Result.Success();
        }

        public override Result ModifyRecv(Packet packet, MemoryStream packetBuf, int packetStartPosition)
        {
            uint len = Protocol.GetMessageLength(packetBuf, packetStartPosition);

            byte[] buf = packetBuf.GetBuffer();
            byte[] decryptedData = recevier.TransformFinalBlock(buf, packetStartPosition + Protocol.headerSize, (int)(len - Protocol.headerSize));
            Buffer.BlockCopy(decryptedData, 0, buf, packetStartPosition + Protocol.headerSize, decryptedData.Length);

            recevier.UpdateHash(decryptedData, 0, decryptedData.Length);

            // 완료 처리 
            recevier.Complete();

            // 패킷 길이 변경
            Protocol.SetMessageLength(packetBuf, packetStartPosition, (uint)(decryptedData.Length + Protocol.headerSize));

            return Result.Success();
        }

        public static string BytesToHexString(byte[] Bytes)
        {
            StringBuilder Result = new StringBuilder(Bytes.Length * 2);
            string HexAlphabet = "0123456789ABCDEF";

            foreach (byte B in Bytes)
            {
                Result.Append(HexAlphabet[(int)(B >> 4)]);
                Result.Append(HexAlphabet[(int)(B & 0xF)]);
            }

            return Result.ToString();
        }

        public static byte[] HexStringToBytes(string Hex)
        {
            byte[] Bytes = new byte[Hex.Length / 2];
            int[] HexValue = new int[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

            for (int x = 0, i = 0; i < Hex.Length; i += 2, x += 1)
            {
                Bytes[x] = (byte)(HexValue[Char.ToUpper(Hex[i + 0]) - '0'] << 4 |
                                  HexValue[Char.ToUpper(Hex[i + 1]) - '0']);
            }

            return Bytes;
        }

    }
}
