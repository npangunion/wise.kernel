using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading;

namespace unit
{
    [TestClass]
    public class TestNetwork
    {
        [TestMethod]
        public void TestListenAndConnect()
        {
            var net = wise.Network.Instance;
            Assert.IsTrue(net.Listen("127.0.0.1:7000"));

            wise.Session session = null;

            net.Subscribe(new wise.SysSessionReady(), (m) => session = m.Session);

            Assert.IsTrue(net.Connect("127.0.0.1:7000"));

            // 그냥 실행은 문제 없으나 디버깅할 경우 타이밍으로 인한 에러 발생 가능
            Thread.Sleep(100);

            Assert.IsTrue(session != null);

            session.Disconnect();

            // 소켓을 닫았을 때 두 개 연결이 모두 종료되어야 하나 안 그런 듯

            Thread.Sleep(100);
        }

        [TestMethod]
        public void TestMessageFactory()
        {

        }

        [TestMethod]
        public void TestSessionProtocol()
        {
            //
            // 에코 정도만 진행하고 C++ 쪽 서버에 붙여서 테스트 한다. 
            // 
        }

        [TestMethod]
        public void TestMessageModifier()
        {

        }
    }
}
