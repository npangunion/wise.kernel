using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace unit
{
    [TestClass]
    public class TestLogger
    {
        [TestMethod]
        public void TestInterface()
        {
            wise.Network.Logger.Info(string.Format("Hello Logger. {0}", 1));
        }
    }
}
