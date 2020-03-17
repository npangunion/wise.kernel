using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    public class Message
    {
        private Topic topic;

        public Message(Topic topic)
        {
            this.topic = topic;
        }

        public Topic Topic
        {
            get { return topic; }
        }
    }
}
