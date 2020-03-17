using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    public class Topic
    {
        private uint key = 0;

        public Topic()
        {
        }

        public Topic(uint key)
        {
            this.key = key;
        }

        public Topic(byte cat, byte group, ushort type)
        {
            key = (uint)(cat << 24 | group << 16 | type);
        }

        public uint Key
        {
            get { return key; }
        }

        public byte Category
        {
            get { return (byte)((key >> 24) & 0x000000FF); }
        }

        public byte Group
        {
            get { return (byte)((key >> 16) & 0x000000FF); }
        }

        public ushort Type
        {
            get { return (ushort)(key & 0x0000FFFF); }
        }

        public bool Valid
        {
            get { return key > 0;  }
        }

        public Topic GetGroupTopic()
        {
            return new Topic(Category, Group, 0);
        }

        public static bool operator == (Topic t1, Topic t2)
        {
            return t1.Key == t2.Key;
        }
        
        public static bool operator != (Topic t1, Topic t2)
        {
            return t1.Key != t2.Key;
        }

        public override int GetHashCode()
        {
            return (int)Key;
        }

        public override bool Equals(Object o)
        {
            return Equals(o as Topic);
        }

        public bool Equals(Topic t)
        {
            return Key == t.Key;
        }

        public override string ToString()
        {
            return string.Format("{0}/{1}/{2}", Category, Group, Type);
        }
    }
}
