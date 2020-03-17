using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Globalization;

namespace wise
{
    public interface IPackable
    {
        bool Pack(Packer packer, Stream stream);

        bool Unpack(Packer packer, Stream Stream);
    }

    /// <summary>
    /// - Little endian 기준으로 수치들 패킹 
    /// - UTF8로 문자열 패킹
    /// - Read는 호출 전에 필요한 만큼 있는 지 확인 필요
    /// </summary>
    public class Packer
    {
        public void Pack(Stream stream, bool b)
        {
            Pack(stream, b ? (sbyte)1 : (sbyte)0);
        }

        private byte[] bout = new byte[1];
        public void Pack(Stream stream, sbyte b)
        {
            bout[0] = (byte)b;
            PackAll(stream, bout, 0, 1);
        }

        private byte[] i16out = new byte[2];
        public void PackShort(Stream stream, short s)
        {
            i16out[1] = (byte)(0xff & (s >> 8));
            i16out[0] = (byte)(0xff & s);
            PackAll(stream, i16out, 0, 2);
        }

        private byte[] i32out = new byte[4];
        public void Pack(Stream stream, int i32)
        {
            i32out[3] = (byte)(0xff & (i32 >> 24));
            i32out[2] = (byte)(0xff & (i32 >> 16));
            i32out[1] = (byte)(0xff & (i32 >> 8));
            i32out[0] = (byte)(0xff & i32);
            PackAll(stream, i32out, 0, 4);
        }

        private byte[] i64out = new byte[8];
        public void Pack(Stream stream, long i64)
        {
            i64out[7] = (byte)(0xff & (i64 >> 56));
            i64out[6] = (byte)(0xff & (i64 >> 48));
            i64out[5] = (byte)(0xff & (i64 >> 40));
            i64out[4] = (byte)(0xff & (i64 >> 32));
            i64out[3] = (byte)(0xff & (i64 >> 24));
            i64out[2] = (byte)(0xff & (i64 >> 16));
            i64out[1] = (byte)(0xff & (i64 >> 8));
            i64out[0] = (byte)(0xff & i64);
            PackAll(stream, i64out, 0, 8);
        }

        public void Pack(Stream stream, byte b)
        {
            bout[0] = (byte)b;
            PackAll(stream, bout, 0, 1);
        }

        public void PackShort(Stream stream, ushort s)
        {
            i16out[1] = (byte)(0xff & (s >> 8));
            i16out[0] = (byte)(0xff & s);
            PackAll(stream, i16out, 0, 2);
        }

        public void Pack(Stream stream, uint i32)
        {
            i32out[3] = (byte)(0xff & (i32 >> 24));
            i32out[2] = (byte)(0xff & (i32 >> 16));
            i32out[1] = (byte)(0xff & (i32 >> 8));
            i32out[0] = (byte)(0xff & i32);
            PackAll(stream, i32out, 0, 4);
        }

        public void Pack(Stream stream, ulong i64)
        {
            i64out[7] = (byte)(0xff & (i64 >> 56));
            i64out[6] = (byte)(0xff & (i64 >> 48));
            i64out[5] = (byte)(0xff & (i64 >> 40));
            i64out[4] = (byte)(0xff & (i64 >> 32));
            i64out[3] = (byte)(0xff & (i64 >> 24));
            i64out[2] = (byte)(0xff & (i64 >> 16));
            i64out[1] = (byte)(0xff & (i64 >> 8));
            i64out[0] = (byte)(0xff & i64);
            PackAll(stream, i64out, 0, 8);
        }

        public void Pack(Stream stream, float d)
        {
            var bytes = BitConverter.GetBytes(d);

            // 여기는 엔디안이 반영될 것으로 보인다. 

            Pack(stream, (sbyte)bytes[0]);
            Pack(stream, (sbyte)bytes[1]);
            Pack(stream, (sbyte)bytes[2]);
            Pack(stream, (sbyte)bytes[3]);
        }

        public void Pack(Stream stream, double d)
        {
            Pack(stream, BitConverter.DoubleToInt64Bits(d));
        }

        public void Pack(Stream stream, string s)
        {
            byte[] sb = Encoding.UTF8.GetBytes(s);

            PackShort(stream, (ushort)sb.Length);
            PackAll(stream, sb, 0, sb.Length);
        }

        public void PackEnum(Stream stream, int v)
        {
            Pack(stream, v);
        }

        public void PackEnum<T>(Stream stream, T v)
        {
            Pack(stream, (int)(object)v);
        }

        public void Pack<T>(Stream stream, T v) where T : IPackable
        {
            v.Pack(this, stream);
        }

        public void Pack(Stream stream, List<bool> lst) 
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<sbyte> lst) 
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<short> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                PackShort(stream, v);
            }
        }

        public void Pack(Stream stream, List<int> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<long> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<byte> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<ushort> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                PackShort(stream, v);
            }
        }

        public void Pack(Stream stream, List<uint> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<ulong> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<float> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<double> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack(Stream stream, List<string> lst)
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);

            foreach (var v in lst)
            {
                Pack(stream, v);
            }
        }

        public void Pack<T>(Stream stream, List<T> lst) where T : IPackable
        {
            ushort len = (ushort)lst.Count;

            PackShort(stream, len);
            
            foreach ( var v in lst )
            {
                v.Pack(this, stream);
            }
        }

        public void Unpack(Stream stream, out bool v)
        {
            sbyte sv;
            Unpack(stream, out sv);

            v = (sv != 0);
        }

        private byte[] bin = new byte[1];
        public void Unpack(Stream stream, out sbyte v)
        {
            UnpackAll(stream, bin, 0, 1);
            v = (sbyte)bin[0];
        }

        private byte[] i16in = new byte[2];
        public void UnpackShort(Stream stream, out short v)
        {
            UnpackAll(stream, i16in, 0, 2);
            v = (short)(((i16in[1] & 0xff) << 8) | ((i16in[0] & 0xff)));
        }

        private byte[] i32in = new byte[4];
        public void Unpack(Stream stream, out int v)
        {
            UnpackAll(stream, i32in, 0, 4);
            v = (int)(((i32in[3] & 0xff) << 24) | ((i32in[2] & 0xff) << 16) | ((i32in[1] & 0xff) << 8) | ((i32in[0] & 0xff)));
        }

#pragma warning disable 675

        private byte[] i64in = new byte[8];
        public void Unpack(Stream stream, out long v)
        {
            UnpackAll(stream, i64in, 0, 8);
            unchecked
            {
                v = (long)(
                    ((long)(i64in[7] & 0xff) << 56) |
                    ((long)(i64in[6] & 0xff) << 48) |
                    ((long)(i64in[5] & 0xff) << 40) |
                    ((long)(i64in[4] & 0xff) << 32) |
                    ((long)(i64in[3] & 0xff) << 24) |
                    ((long)(i64in[2] & 0xff) << 16) |
                    ((long)(i64in[1] & 0xff) << 8) |
                    ((long)(i64in[0] & 0xff)));
            }
        }

#pragma warning restore 675

        public void Unpack(Stream stream, out byte v)
        {
            sbyte wv;
            Unpack(stream, out wv);
            v = (byte)wv;
        }

        public void UnpackShort(Stream stream, out ushort v)
        {
            short wv;
            UnpackShort(stream, out wv);
            v = (ushort)wv;
        }

        public void Unpack(Stream stream, out uint v)
        {
            int wv;
            Unpack(stream, out wv);
            v = (uint)wv;
        }

        public void Unpack(Stream stream, out ulong v)
        {
            long wv;
            Unpack(stream, out wv);
            v = (ulong)wv;
        }

        public void Unpack(Stream stream, out float v)
        {
            UnpackAll(stream, i32in, 0, 4);

            v = BitConverter.ToSingle(i32in, 0);
        }

        public void Unpack(Stream stream, out double v)
        {
            long tv = 0;
            Unpack(stream, out tv);
            v = BitConverter.Int64BitsToDouble(tv);
        }

        public void Unpack(Stream stream, out string s)
        {
            ushort len;

            UnpackShort(stream, out len);

            if ( len == 0 )
            {
                s = "";
                return;
            }

            byte[] sb = new byte[len];

            UnpackAll(stream, sb, 0, len);

            s = Encoding.UTF8.GetString(sb);
        }

        public void UnpackEnum<T>(Stream stream, out T val) where T : struct, IConvertible
        {
            if (!typeof(T).IsEnum)
            {
                throw new ArgumentException(string.Format("{0} is not enum", nameof(val)));
            }

            int v;

            Unpack(stream, out v);

            if (Enum.IsDefined(typeof(T), v))
            {
                val = (T)(object)v;
            }
            else
            {
                throw new ArgumentOutOfRangeException("Enum value is out of range");
            }
        }

        public void Unpack<T>(Stream stream, out T v) where T : IPackable, new()
        {
            T val = new T();
            val.Unpack(this, stream);
            v = val;
        }

        public void Unpack(Stream stream, List<bool> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for ( ushort i=0; i<len; ++i)
            {
                bool v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<sbyte> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                sbyte v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<short> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                short v;
                UnpackShort(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<int> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                int v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<long> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                long v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<byte> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                byte v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<ushort> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                ushort v;
                UnpackShort(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<uint> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                uint v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<ulong> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                ulong v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<float> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                float v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<double> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                double v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack(Stream stream, List<string> lst)
        {
            ushort len;

            UnpackShort(stream, out len);

            for (ushort i = 0; i < len; ++i)
            {
                string v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        public void Unpack<T>(Stream stream, List<T> lst) where T : IPackable, new()
        {
            ushort len;

            UnpackShort(stream, out len);

            for ( ushort i = 0; i<len; ++i)
            {
                T v;
                Unpack(stream, out v);
                lst.Add(v);
            }
        }

        private void PackAll(Stream stream, byte[] buf, int off, int len)
        {
            stream.Write(buf, off, len);
        }

        private int UnpackAll(Stream stream, byte[] buf, int off, int len)
        {
            return stream.Read(buf, 0, len);
        }
    }
}
