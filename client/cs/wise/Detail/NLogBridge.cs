using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NLog;

namespace wise.Detail
{
    internal class NLogBridge : wise.ILogger 
    {
        private static NLog.Logger logger = NLog.LogManager.GetCurrentClassLogger();

        public void Trace(string s)
        {
            logger.Trace(s); 
        }

        public void Debug(string s)
        {
            logger.Debug(s); 
        }

        public void Info(string s)
        {
            logger.Info(s); 
        }

        public void Warn(string s)
        {
            logger.Warn(s); 
        }

        public void Error(string s)
        {
            logger.Error(s);
        }

        public void Fatal(string s)
        {
            logger.Fatal(s);
        }
    }
}
