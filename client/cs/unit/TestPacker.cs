using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using System.Collections.Generic;

namespace unit
{
    enum TestGeneral
    {
        V1, 
        V2,
        V3,
        V4
    }

    class Item : wise.IPackable
    {
       public int id;
       public string name;

        public bool Pack(wise.Packer packer, Stream stream)
        {
            packer.Pack(stream, id);
            packer.Pack(stream, name);
            return true;
        }

        public bool Unpack(wise.Packer packer, Stream stream)
        {
            packer.Unpack(stream, out id);
            packer.Unpack(stream, out name);
            return true;
        }
    }

    [TestClass]
    public class TestPacker
    {
        [TestMethod]
        public void TestPackNumericTypes()
        {
            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            packer.Pack(stream, true);
            packer.Pack(stream, (sbyte)1);
            packer.PackShort(stream, 2);
            packer.Pack(stream, (long)3);
            packer.Pack(stream, (int)4);

            packer.Pack(stream, (byte)5);
            packer.PackShort(stream, (ushort)6);
            packer.Pack(stream, (ulong)7);
            packer.Pack(stream, (uint)8);

            packer.Pack(stream, 9.1f);
            packer.Pack(stream, 10.1);

            bool bv;
            sbyte bbv;
            short sv;
            long lv;
            int iv;

            byte ubbv;
            ushort usv;
            ulong ulv;
            uint uiv;

            float fv;
            double dv;

            stream.Position = 0;

            packer.Unpack(stream, out bv);
            packer.Unpack(stream, out bbv);
            packer.UnpackShort(stream, out sv);
            packer.Unpack(stream, out lv);
            packer.Unpack(stream, out iv);

            packer.Unpack(stream, out ubbv);
            packer.UnpackShort(stream, out usv);
            packer.Unpack(stream, out ulv);
            packer.Unpack(stream, out uiv);

            packer.Unpack(stream, out fv);
            packer.Unpack(stream, out dv);

            Assert.IsTrue(bv == true);
            Assert.IsTrue(bbv == 1);
            Assert.IsTrue(sv == 2);
            Assert.IsTrue(lv == 3);
            Assert.IsTrue(iv == 4);

            Assert.IsTrue(ubbv == 5);
            Assert.IsTrue(usv == 6);
            Assert.IsTrue(ulv == 7);
            Assert.IsTrue(uiv == 8);

            Assert.IsTrue(fv == 9.1f);
            Assert.IsTrue(dv == 10.1);
        }

        [TestMethod]
        public void TestPackString()
        {
            string s = "Hello 한글 포함 world!";

            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            packer.Pack(stream, s);

            stream.Position = 0;

            string s2;

            packer.Unpack(stream, out s2);

            Assert.IsTrue(s == s2);
        }

        [TestMethod]
        public void TestPackEnum()
        {
            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            packer.PackEnum(stream, TestGeneral.V2);

            stream.Position = 0;

            TestGeneral tg;

            packer.UnpackEnum<TestGeneral>(stream, out tg);

            Assert.IsTrue(tg == TestGeneral.V2);
        }

        [TestMethod]
        public void TestPackList()
        {
            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            // bool list
            {
                var blst = new List<bool>();

                blst.Add(true);
                blst.Add(false);
                blst.Add(true);

                packer.Pack(stream, blst);

                stream.Position = 0;

                var nlst = new List<bool>();

                packer.Unpack(stream, nlst);

                Assert.IsTrue(nlst[1] == false);
            }

            stream.Position = 0;

            // float list
            {
                var blst = new List<float>();

                blst.Add(1.0f);
                blst.Add(3.1f);
                blst.Add(2.0f);

                packer.Pack(stream, blst);

                stream.Position = 0;

                var nlst = new List<float>();

                packer.Unpack(stream, nlst);

                Assert.IsTrue(nlst[1] == 3.1f);
            }
        }

        [TestMethod]
        public void TestDictionary()
        {
            // Thrift는 코드 생성 기능으로 대체하고 있다. 
            // 무리하지 말고 그렇게 한다. 
            // 코드 생성 후 확인하다. 
            // 지원하지 않아도 된다. 
            // 선택으로 남기고 나중에 더 보자
        }

        [TestMethod]
        public void TestPackable()
        {
            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            var item = new Item();
            item.id = 10010;
            item.name = "황금사자 갑옷";

            packer.Pack(stream, item);

            stream.Position = 0;

            Item nitem;

            packer.Unpack(stream, out nitem);

            Assert.IsTrue(nitem.id == item.id);
            Assert.IsTrue(nitem.name == item.name);
        }

        [TestMethod]
        public void TestPackableList()
        {
            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            var blst = new List<Item>();

            var item = new Item();
            item.id = 10010;
            item.name = "황금사자 갑옷";

            blst.Add(item);

            var item2 = new Item();
            item2.id = 10011;
            item2.name = "황금사자 수염";
            blst.Add(item2);

            packer.Pack(stream, blst);

            stream.Position = 0;

            var nlst = new List<Item>();

            packer.Unpack(stream, nlst);

            Assert.IsTrue(nlst.Count == 2);

            Assert.IsTrue(nlst[0].id == 10010);

            Assert.IsTrue(nlst[1].id == 10011);
            Assert.IsTrue(nlst[1].name == item2.name);
        } 

        [TestMethod]
        public void TestPackDate()
        {
            wise.Date date = new wise.Date();

            date.year = 2018;
            date.month = 12;
            date.day = 20;

            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            packer.Pack(stream, date);

            stream.Position = 0;

            wise.Date ndate;

            packer.Unpack(stream, out ndate);

            Assert.IsTrue(ndate.year == date.year);
            Assert.IsTrue(ndate.month == date.month );
            Assert.IsTrue(ndate.day == date.day );
        }

        [TestMethod]
        public void TestPackTimestamp()
        {
            wise.Timestamp tms = new wise.Timestamp();

            tms.year = 2018;
            tms.month = 12;
            tms.day = 20;
            tms.hour = 10;
            tms.min = 32;
            tms.sec = 30;
            tms.fract = 103000;

            MemoryStream stream = new MemoryStream();
            wise.Packer packer = new wise.Packer();

            packer.Pack(stream, tms);

            stream.Position = 0;

            wise.Timestamp ntms;

            packer.Unpack(stream, out ntms);

            Assert.IsTrue(ntms.year == tms.year);
            Assert.IsTrue(ntms.month == tms.month);
            Assert.IsTrue(ntms.day == tms.day);
            Assert.IsTrue(ntms.hour == tms.hour );
            Assert.IsTrue(ntms.min == tms.min);
            Assert.IsTrue(ntms.sec == tms.sec);
            Assert.IsTrue(ntms.fract == tms.fract);
        }
    }
}
