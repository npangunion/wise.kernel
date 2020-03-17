using System;
using System.IO;
using System.Collections.Generic;
using wise;

namespace common {

public enum TestEnum
{
	V1,
	V2,
	V3 = 100,
}


public class hello : IPackable 
{
	public uint v = 1;
	public short iv = 3;
	public List<uint> ids = new List<uint>();
	public TestEnum test;

	public bool Pack(Packer packer, Stream stream) 
	{
		packer.Pack(stream, v);
		packer.PackShort(stream, iv);
		packer.Pack(stream, ids);
		packer.PackEnum(stream, test);
		return true;
	}

	public bool Unpack(Packer packer, Stream stream) 
	{
		packer.Unpack(stream,  out v);
		packer.UnpackShort(stream,  out iv);
		packer.Unpack(stream, ids);
		packer.UnpackEnum(stream,  out test);
		return true;
	}
}


} // common


