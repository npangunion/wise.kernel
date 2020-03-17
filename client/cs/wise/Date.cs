using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace wise
{
    public class Date : IPackable
    {
        public short year;
        public short month;
        public short day;  

        public bool Pack(Packer packer, Stream stream)
        {
            packer.PackShort(stream, year);
            packer.PackShort(stream, month);
            packer.PackShort(stream, day);
            return true;
        }

        public bool Unpack(Packer packer, Stream stream)
        {
            packer.UnpackShort(stream, out year);
            packer.UnpackShort(stream, out month);
            packer.UnpackShort(stream, out day);
            return true;
        }
    }

    public class Timestamp : IPackable
    {
        public short year;   //!< Year [0-inf).
        public short month;  //!< Month of the year [1-12].
        public short day;    //!< Day of the month [1-31].
        public short hour;   //!< Hours since midnight [0-23].
        public short min;    //!< Minutes after the hour [0-59].
        public short sec;    //!< Seconds after the minute.
        public int fract;   //!< Fractional seconds.

        public bool Pack(Packer packer, Stream stream)
        {
            packer.PackShort(stream, year);
            packer.PackShort(stream, month);
            packer.PackShort(stream, day);
            packer.PackShort(stream, hour);
            packer.PackShort(stream, min);
            packer.PackShort(stream, sec);
            packer.Pack(stream, fract);
            return true;
        }

        public bool Unpack(Packer packer, Stream stream)
        {
            packer.UnpackShort(stream, out year);
            packer.UnpackShort(stream, out month);
            packer.UnpackShort(stream, out day);
            packer.UnpackShort(stream, out hour);
            packer.UnpackShort(stream, out min);
            packer.UnpackShort(stream, out sec);
            packer.Unpack(stream, out fract);
            return true;
        }
    };
}

