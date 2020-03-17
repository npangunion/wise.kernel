using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace unit
{
    class MessageTest : wise.Message
    {
        public int value = 3;

        public MessageTest()
            : base(new wise.Topic(1, 1, 1))
        {
        }

        public void OnMessage(MessageTest m)
        {
            ++value;
        }
    }

    [TestClass]
    public class TestChannel
    {
        [TestMethod]
        public void TestSequence()
        {
            wise.Detail.Sequence seq = new wise.Detail.Sequence();

            Assert.IsTrue(seq.Next() == 1);
            Assert.IsTrue(seq.Next() == 2);

            for ( int i=0; i<30; ++i)
            {
                seq.Next(); // skip first 32 sequences
            }

            seq.Release(2);

            // unit test tests interface. 
        }

        [TestMethod] 
        public void TestChannelInterface()
        {
            wise.Detail.Channel channel = new wise.Detail.Channel();

            var mt = new MessageTest();
            var tk = channel.Subscribe(mt, mt.OnMessage);

            channel.Post(mt);
            Assert.IsTrue(mt.value == 4);

            channel.Post(mt);
            Assert.IsTrue(mt.value == 5); // posted

            var rc = channel.Unsubscribe(tk);
            Assert.IsTrue(rc); // unsubscribed

            channel.Post(mt);
            Assert.IsTrue(mt.value == 5); // not posted 
        }
    }
}
