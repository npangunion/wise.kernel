#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/protocol/bits/bits_packet.hpp>
#include <wise.kernel/net/protocol/bits/bits_packer.hpp>

using namespace wise::kernel;

namespace
{

struct item : public ipackable
{
	bool pack(bits_packer& packer) const override
	{
		return packer.pack(id);
	}

	bool unpack(bits_packer& packer) override
	{
		return packer.unpack(id);
	}

	uint32_t id = 1;
};

struct vec
{
	float x, y, z;
};

}

namespace wise {
namespace kernel {

template <> bool pack(bits_packer& packer, const vec& v)
{
	packer.pack(v.x);
	packer.pack(v.y);
	packer.pack(v.z);

	return packer.is_valid();
}

template <> bool unpack(bits_packer& packer, vec& v)
{
	packer.unpack(v.x);
	packer.unpack(v.y);
	packer.unpack(v.z);

	return packer.is_valid();
}

} // kernel
} // wise

TEST_CASE("bits packer")
{
	SECTION("resize buffer read interface")
	{
		// append�� �ִµ� read�� iterator / at �ۿ� ��� ��ġ �����ϴ� read�� �ʿ� 

		// pack / unpack �����ϸ鼭 �߰��ϰ� 
		// ���⼭ ���� �׽�Ʈ. 
	}

	SECTION("message pack / unpack")
	{
		resize_buffer buf;

		bits_packer packer(buf);

		SECTION("numerals")
		{
			buf.rewind();

			packer.pack((uint8_t)1);
			packer.pack((uint16_t)2);
			packer.pack((uint32_t)3);
			packer.pack((uint64_t)4);

			packer.pack((int8_t)5);
			packer.pack((int16_t)6);
			packer.pack((int32_t)7);
			packer.pack((int64_t)8);

			uint8_t ui8;
			uint16_t ui16;
			uint32_t ui32;
			uint64_t ui64;

			int8_t i8;
			int16_t i16;
			int32_t i32;
			int64_t i64;

			packer.unpack(ui8);
			packer.unpack(ui16);
			packer.unpack(ui32);
			packer.unpack(ui64);

			packer.unpack(i8);
			packer.unpack(i16);
			packer.unpack(i32);
			packer.unpack(i64);

			CHECK(ui8 == 1);
			CHECK(ui16 == 2);
			CHECK(ui32 == 3);
			CHECK(ui64 == 4);

			CHECK(i8 == 5);
			CHECK(i16 == 6);
			CHECK(i32 == 7);
			CHECK(i64 == 8);
		}

		SECTION("bool")
		{
			// bool ��. ����Ʈ�� ����. 

			buf.rewind();

			packer.pack(true);

			bool bv;

			packer.unpack(bv);

			CHECK(bv == true);
		}

		SECTION("reals")
		{
			buf.rewind();

			packer.pack(0.3f);
			packer.pack(0.5);

			float f;
			double d;

			packer.unpack(f);
			packer.unpack(d);

			CHECK(is_equal(0.3f, f));
			CHECK(is_equal(0.5, d));
		}

		SECTION("struct. ipackable")
		{
			buf.rewind();

			item iv;

			iv.pack(packer);

			iv.id = 0;

			iv.unpack(packer);

			CHECK(iv.id == 1);

			buf.rewind();

			packer.pack(iv);

			iv.id = 0;
			packer.unpack(iv);

			CHECK(iv.id == 1);
		}

		SECTION("vector")
		{
			std::vector<uint32_t> vec;

			vec.push_back(1);
			vec.push_back(2);
			vec.push_back(3);
			vec.push_back(4);

			buf.rewind();

			packer.pack(vec);

			vec.clear();

			packer.unpack(vec);

			CHECK(vec[0] == 1);
			CHECK(vec[3] == 4);
		}

		SECTION("map")
		{
			std::map<int, int> mv;

			mv[0] = 1;
			mv[1] = 2;
			mv[2] = 3;

			buf.rewind();

			packer.pack(mv);

			mv.clear();

			packer.unpack(mv);

			CHECK(mv[0] == 1);
			CHECK(mv[2] == 3);
		}

		SECTION("string")
		{
			std::string hello("hello zen!");

			buf.rewind();

			packer.pack(hello);

			hello.clear();

			packer.unpack(hello);

			CHECK(hello == "hello zen!");
		}

		SECTION("enum")
		{
			// enum ���� �ٸ� ����� �ʿ��ϴ�. 
			// c#�� c++ ���� �޸𸮷� �� �� ����, ������ �ؾ� �Ѵ�. 
			// �Ƹ��� int32�� �����ؾ� ���� ������? 
			// 

			enum test_enum
			{
				v1,
				v2,
				v3
			};

			buf.rewind();

			packer.pack_enum(test_enum::v1);

			test_enum v;
			packer.unpack_enum(v);

			CHECK(v == test_enum::v1);

			// ������ ������ int8_t Ÿ���� enum �� �����Ѵ�. 
			// ������ enum ���� ������ ���� ��
		}

		SECTION("wstring")
		{
			std::wstring hello(L"hello zen!");

			buf.rewind();

			packer.pack(hello);

			hello.clear();

			packer.unpack(hello);

			CHECK(hello == L"hello zen!");
		}

		SECTION("extension")
		{
			vec v;

			v.x = 1.0f;
			v.y = 2.0f;
			v.z = 3.0f;

			buf.rewind();

			packer.pack(v);

			vec ov; 

			packer.unpack(ov);

			CHECK(is_equal(ov.x, 1.0f));
			CHECK(is_equal(ov.y, 2.0f));
			CHECK(is_equal(ov.z, 3.0f));
		}
	}
}