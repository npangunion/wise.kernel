using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace unit
{
    class PacketSample : wise.Packet
    {
        public int value = 3;

        public PacketSample()
            : base(new wise.Topic(1, 1, 1))
        {
        }
    }

    class PacketSeq : wise.Packet
    {
        public int value = 3;

        public PacketSeq()
            : base(new wise.Topic(1, 1, 2))
        {
            IsSequenceEnabled = true;
        }
    }

    class PacketChecksum: wise.Packet
    {
        public int value = 3;

        public PacketChecksum()
            : base(new wise.Topic(1, 1, 3))
        {
            IsSequenceEnabled = true;
            IsChecksumEnabled = true;
        }

        public override bool Pack(wise.Packer packer, Stream stream)
        {
            packer.Pack(stream, value);
            return true;
        }

        public override bool Unpack(wise.Packer packer, Stream stream)
        {
            packer.Unpack(stream, out value);
            return true;
        }
    }

    class PacketCipher : wise.Packet
    {
        public int value = 0;
        public string hello = "Hello Cipher Modifier. Good luck!";

        public PacketCipher()
            : base(new wise.Topic(1, 1, 4))
        {
            IsSequenceEnabled = true;
            IsChecksumEnabled = true;
            IsCipherEnabled = true;
        }

        public override bool Pack(wise.Packer packer, Stream stream)
        {
            packer.Pack(stream, value);
            packer.Pack(stream, hello);
            return true;
        }

        public override bool Unpack(wise.Packer packer, Stream stream)
        {
            packer.Unpack(stream, out value);
            packer.Unpack(stream, out hello);
            return true;
        }
    }

    [TestClass]
    public class TestProtocol
    {
        [TestMethod]
        public void TestProtocolPacketization()
        {
            int recvCount = 0;

            wise.Network.Instance.Subscribe(new PacketSample(), (m) => recvCount++);
            wise.PacketFactory.Instance.Add(new wise.Topic(1, 1, 1), () => { return new PacketSample(); });
            wise.Detail.Protocol protocol = new wise.Detail.Protocol();

            PacketSample ps = new PacketSample();
            MemoryStream ms = new MemoryStream();

            protocol.OnSend(ps, ms);

            ms.Position = 0;

            protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);
            Assert.IsTrue(recvCount == 1);

            protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);
            Assert.IsTrue(recvCount == 2);
        }

        [TestMethod]
        public void TestProtocolPartialRecv()
        {
            int recvCount = 0;

            wise.Network.Instance.Subscribe(new PacketSample(), (m) => recvCount++);
            wise.PacketFactory.Instance.Add(new wise.Topic(1, 1, 1), () => { return new PacketSample(); });
            wise.Detail.Protocol protocol = new wise.Detail.Protocol();

            PacketSample ps = new PacketSample();
            MemoryStream ms = new MemoryStream();

            protocol.OnSend(ps, ms);

            ms.Position = 0;

            protocol.OnRecv(ms.GetBuffer(), (int)(ms.Length-5));
            Assert.IsTrue(recvCount == 0);

            MemoryStream rs = new MemoryStream();

            rs.Write(ms.GetBuffer(), (int)(ms.Length - 5), 5);
            rs.Write(ms.GetBuffer(), 0, (int)ms.Length);

            protocol.OnRecv(rs.GetBuffer(), (int)rs.Length);
            Assert.IsTrue(recvCount == 2);
        }

        [TestMethod]
        public void TestProtocolSequenceModifier()
        {
            // 순서대로 하나씩 옵션을 추가하면서 테스트

            int recvCount = 0;

            wise.Network.Instance.Subscribe(new PacketSeq(), (m) => recvCount++);
            wise.PacketFactory.Instance.Add(new wise.Topic(1, 1, 2), () => { return new PacketSeq(); });
            wise.Detail.Protocol protocol = new wise.Detail.Protocol();

            PacketSeq ps = new PacketSeq();
            MemoryStream ms = new MemoryStream();

            for (int i = 0; i < 1024; ++i)
            {
                ms.Position = 0;
                protocol.OnSend(ps, ms);

                var result = protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);

                Assert.IsTrue(!!result);
            }
        }

        [TestMethod]
        public void TestProtocolChecksumModifier()
        {
            int recvCount = 0;

            wise.Network.Instance.Subscribe(new PacketChecksum(), (m) => recvCount++);
            wise.PacketFactory.Instance.Add(new wise.Topic(1, 1, 3), () => { return new PacketChecksum(); });
            wise.Detail.Protocol protocol = new wise.Detail.Protocol();

            PacketChecksum ps = new PacketChecksum();
            MemoryStream ms = new MemoryStream();

            for (int i = 0; i < 1024; ++i)
            {
                ms.Position = 0;
                protocol.OnSend(ps, ms);

                var result = protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);

                Assert.IsTrue(!!result);
            }
        }

        [TestMethod]
        public void TestHexToString()
        {
            var org = "2B7E151628AED2A6ABF7158809CF4F3C";
            var bytes = wise.Detail.CipherModifier.HexStringToBytes(org);
            var str = wise.Detail.CipherModifier.BytesToHexString(bytes);

            Assert.IsTrue(org == str);
        }

        [TestMethod]
        public void TestProtocolCipherModifier()
        {
            int recvCount = 0;

            PacketCipher pc = null;

            wise.Network.Instance.Subscribe(
                new PacketCipher(),
                (m) => {
                    recvCount++;
                    wise.Network.Logger.Info(string.Format("{0}", m.hello));
                    pc = m;
                }
                );
            wise.PacketFactory.Instance.Add(new wise.Topic(1, 1, 4), () => { return new PacketCipher(); });
            wise.Detail.Protocol protocol = new wise.Detail.Protocol();

            PacketCipher ps = new PacketCipher();
            MemoryStream ms = new MemoryStream();

            ps.value = 77;

            for (int i = 0; i < 128; ++i)
            {
                ps.hello = ps.hello.Insert(0, "Hello ");
            }

            for (int i = 0; i < 2048; ++i)
            {
                ms.Position = 0;
                protocol.OnSend(ps, ms);
                // protocol.OnSend(ps, ms);

                var result = protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);
                Assert.IsTrue(result);
            }

            Assert.IsTrue(pc != null);
            Assert.IsTrue(pc.value == ps.value);
            Assert.IsTrue(pc.hello == ps.hello);

            // ms.Position = 0;
            // protocol.OnSend(ps, ms);
            // protocol.OnSend(ps, ms);

            // result = protocol.OnRecv(ms.GetBuffer(), (int)ms.Length);
            // Assert.IsTrue(result);
        }
    }
}
