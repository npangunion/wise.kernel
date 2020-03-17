using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

/** 
 * Some part of Handler, Channel implementations are borrowed from 
 * x3net by JayKang. It is under MIT license.
 */

namespace wise.Detail
{
    /// <summary>
    /// Topic based subscription and posting channel.
    /// ReaderWriterLockSlim을 사용. 
    /// 제약은 없으나 Subscribe를 앱 시작 시 한번만 하는 것을 권고
    /// </summary>
    internal class Channel
    {
        class Subscription
        {
            public int id;
            public Handler handler;
        }

        private Dictionary<Topic, List<Subscription>> subscriptions;
        private Dictionary<int, Topic> indexTopic;
        private ReaderWriterLockSlim rwlock;
        private Sequence sequence;

        public Channel()
        {
            subscriptions = new Dictionary<Topic, List<Subscription>>();
            indexTopic = new Dictionary<int, Topic>();
            rwlock = new ReaderWriterLockSlim();
            sequence = new Sequence();
        }

        public int Post(Message m)
        {
            int postedCount = 0;

            rwlock.EnterReadLock();

            try
            {
                List<Subscription> subs;

                if (subscriptions.TryGetValue(m.Topic, out subs))
                {
                    foreach (var sub in subs)
                    {
                        sub.handler.Invoke(m);

                        ++postedCount;
                    }
                }
            }
            finally
            {
                rwlock.ExitReadLock();
            }

            return postedCount;
        }

        /**
         * Subscribe. 
         */
        public Token Subscribe<T>(T m, Action<T> action) where T : Message
        {
            rwlock.EnterWriteLock();

            int id = 0;

            try
            {
                id = Subscribe(m, new MethodHandler<T>(action));
            }
            finally
            {
                rwlock.ExitWriteLock();
            }

            Network.Logger.Trace( string.Format("Subscribe to Topic: {0}, Hanlder: {1}", m.Topic, action) );

            return new Token(id, m);
        }

        public bool Unsubscribe(Token token)
        {
            if (token.Valid)
            {
                return Unsubscribe(token.Key);
            }

            return false;
        }

        private bool Unsubscribe(int id)
        {
            rwlock.EnterWriteLock();

            bool result = false;

            try
            {
                Topic topic;

                if (indexTopic.TryGetValue(id, out topic))
                {
                    List<Subscription> subs;

                    if (subscriptions.TryGetValue(topic, out subs))
                    {
                        for (int i = 0; i < subs.Count; ++i)
                        {
                            if (subs[i].id == id)
                            {
                                subs.RemoveAt(i);
                                result = true;
                                break;
                            }
                        }
                    }
                }
            }
            finally
            {
                rwlock.ExitWriteLock();
            }

            sequence.Release(id);

            return result;
        }

        int Subscribe(Message m, Handler handler)
        {
            List<Subscription> subs;

            Subscription sub = new Subscription();
            sub.id = sequence.Next();
            sub.handler = handler;

            if (subscriptions.TryGetValue(m.Topic, out subs))
            {
                subs.Add(sub);
            }
            else
            {
                var nsubs = new List<Subscription>();
                nsubs.Add(sub);
                subscriptions.Add(m.Topic, nsubs);
            }

            indexTopic.Add(sub.id, m.Topic);

            return sub.id;
        } 
    }
}
