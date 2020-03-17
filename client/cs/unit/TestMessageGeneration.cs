using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace unit
{
    [TestClass]
    public class TestMessageGeneration
    {
        [TestMethod]
        public void TestGeneratedMessage()
        {
            var req = new shop.req_buy_item();

            var item1 = new shop.item();

            item1.id = 5;
            item1.he.iv = 10;
            item1.he.ids.Add(3);
            item1.he.test = common.TestEnum.V2;
            item1.name = "Item 1";

            req.items.Add(item1);

            var item2 = new shop.item();

            item2.id = 6;
            item2.he.iv = 10;
            item2.he.ids.Add(3);
            item2.he.ids.Add(4);
            item2.he.test = common.TestEnum.V3;
            item2.name = "Item 2";

            req.items.Add(item2);

            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            req.Pack(packer, stream);

            stream.Position = 0;

            var res = new shop.req_buy_item();

            res.Unpack(packer, stream);

            Assert.IsTrue(res.items.Count == 2);
            Assert.IsTrue(res.items[0].he.test == common.TestEnum.V2);
            Assert.IsTrue(res.items[0].name == "Item 1");

            Assert.IsTrue(res.items[1].he.test == common.TestEnum.V3);
            Assert.IsTrue(res.items[1].name == "Item 2");
            Assert.IsTrue(res.items[1].id == 6);
            Assert.IsTrue(res.items[1].he.ids[1] == 4);
        }
    }
}
