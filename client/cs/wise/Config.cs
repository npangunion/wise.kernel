using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wise
{
    public class Config
    {
        public static int MaxPacketSize = 512 * 1024;

        public static bool EnableSequence = true;
        public static bool EnableChecksum = true;
        public static bool EnableCipher = true;

        public static int CipherKeyChangeInterval = 1024;
    }
}
