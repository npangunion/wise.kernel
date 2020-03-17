using System;
using System.IO;
using System.Collections.Generic;
using wise;


namespace shop {

public enum types
{
	v1,
	v2 = 1,
	v3,
}


public class item : IPackable 
{
	public uint id = 0;
#if _DEBUG
	public int v = 5;
#else 
	public double v = 5;
#endif
	public float key = 0;
	public string name = "";
	public string desc = "";
	public common.hello he = new common.hello();
	public common.TestEnum te;

	public bool Pack(Packer packer, Stream stream) 
	{
		packer.Pack(stream, id);
#if _DEBUG
		packer.Pack(stream, v);
#else 
		packer.Pack(stream, v);
#endif
		packer.Pack(stream, key);
		packer.Pack(stream, name);
		packer.Pack(stream, desc);
		packer.Pack(stream, he);
		packer.PackEnum(stream, te);
		return true;
	}

	public bool Unpack(Packer packer, Stream stream) 
	{
		packer.Unpack(stream,  out id);
#if _DEBUG
		packer.Unpack(stream,  out v);
#else 
		packer.Unpack(stream,  out v);
#endif
		packer.Unpack(stream,  out key);
		packer.Unpack(stream,  out name);
		packer.Unpack(stream,  out desc);
		packer.Unpack(stream,  out he);
		packer.UnpackEnum(stream,  out te);
		return true;
	}
}


public class req_buy_item : Packet
{
	public List<item> items = new List<item>();
#if _DEBUG
	public types v1;
#endif


	public static Topic GetTopic() 
	{
		return new Topic( (byte)play.shop.category, (byte)play.shop.group, (ushort)play.shop.req_buy_item );
	}

	public req_buy_item()
	: base(GetTopic())
	{
	}

	public override bool Pack(Packer packer, Stream stream) 
	{
		packer.Pack(stream, items);
#if _DEBUG
		packer.PackEnum(stream, v1);
#endif
		return true;
	}

	public override bool Unpack(Packer packer, Stream stream) 
	{
		packer.Unpack(stream, items);
#if _DEBUG
		packer.UnpackEnum(stream, out v1);
#endif
		return true;
	}
}



} // shop


