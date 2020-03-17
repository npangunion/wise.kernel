using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace console
{
    class Program
    {
        enum Code
        {
            A, 
            B, 
            C
        }

        static void Main(string[] args)
        {
            Experiment();
        }

        static void Experiment()
        {
            wise.Network.Logger.Info(string.Format("Hello Logger {0}", 1));
            wise.Network.Logger.Info(Code.A.ToString("g"));
        }


    }

}
