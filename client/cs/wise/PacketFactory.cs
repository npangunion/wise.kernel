using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    /// <summary>
    /// 클라이언트 시작 시 모두 Add로 등록하는 걸 가정. 
    /// </summary>
    public class PacketFactory
    {
        private Dictionary<Topic, Func<Packet>> creators;

        public static PacketFactory Instance
        {
            get;
            private set;
        }

        /// <summary>
        /// setup static properites
        /// </summary>
        static PacketFactory()
        {
            Instance = new PacketFactory();
        }

        private PacketFactory()
        {
            creators = new Dictionary<Topic, Func<Packet>>();
        }

        public void Add(Topic topic, Func<Packet> func)
        {
            if ( creators.ContainsKey(topic) )
            {
                return;
            }

            creators.Add(topic, func);
        }

        public Packet Create(Topic topic)
        {
            Func<Packet> creator;

            if (creators.TryGetValue(topic, out creator))
            {
                return creator();
            }

            return null;
        }
    }
}
