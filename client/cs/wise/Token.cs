using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    public class Token
    {
        int key;
        Message m;

        public Token( int key, Message m )
        {
            this.key = key;
            this.m = m;
        }

        public int Key
        {
            get { return key; }
        }

        public bool Valid
        {
            get { return key > 0 && m != null; }
        }

        public void Reset()
        {
            Network.Instance.Unsubscribe(this);
        }
    }
}
