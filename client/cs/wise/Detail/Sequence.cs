using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise.Detail
{
    /// <summary>
    /// 1 to int.MaxValue sequence pool
    /// </summary>
    internal class Sequence
    {
        private object lock_obj;
        private Queue<int> pool;
        private int current;

        public Sequence()
        {
            lock_obj = new object();
            pool = new Queue<int>();
            current = 0;
        }

        public int Next()
        {
            lock (lock_obj)
            {
                if ( pool.Count == 0)
                {
                    Reserve();
                }

                return pool.Dequeue();
            }
        }

        public void Release(int id)
        {
            lock (lock_obj)
            {
                pool.Enqueue(id);
            }
        }

        private void Reserve()
        {
            for ( int i=0; i<16; ++i)
            {
                if ( current == int.MaxValue)
                {
                    throw new ArgumentOutOfRangeException(
                        "Sequence.current", "sequenc cannot reach max integer value"
                        );
                }

                pool.Enqueue(++current);
            }
        }
    }
}
