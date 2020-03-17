using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    /// <summary>
    /// Use string.format() to pass a log info
    /// </summary>
    public interface ILogger
    {
        void Trace(string s);

        void Debug(string s);

        void Info(string s);

        void Warn(string s);

        void Error(string s);

        void Fatal(string s);
    }
}
