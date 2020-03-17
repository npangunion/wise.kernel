using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

/** 
 * Some part of Handler, Channel implementations are borrowed from 
 * x3net by JayKang. It is under MIT license.
 */

namespace wise
{
    /// <summary>
    /// Abstract base class for concrete event handlers.
    /// </summary>
    public abstract class Handler
    {
        /// <summary>
        /// Gets the underlying delegate of this handler.
        /// </summary>
        public abstract Delegate Action { get; }

        /// <summary>
        /// Determines whether the specified object is equal to the current
        /// object.
        /// </summary>
        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }
            if (obj == null || GetType() != obj.GetType())
            {
                return false;
            }

            Handler other = (Handler)obj;
            return Action.Equals(other.Action);
        }

        /// <summary>
        /// Returns the hash code for the current object.
        /// </summary>
        public override int GetHashCode()
        {
            return Action.GetHashCode();
        }

        /// <summary>
        /// Invokes the underlying delegate of this handler with the specified
        /// event.
        /// </summary>
        public abstract void Invoke(Message m);
    }

    /// <summary>
    /// Represents a generic method handler.
    /// </summary>
    public class MethodHandler<T> : Handler
        where T : Message
    {
        protected readonly Action<T> action;

        public override Delegate Action { get { return action; } }

        public MethodHandler(Action<T> action)
        {
            this.action = action;
        }

        public override void Invoke(Message m)
        {
            action((T)m);
        }
    }

    /**
     * x2net has PredicateHandler and others 
     * to filter on conditions and so on.
     */
}
