#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/buffer/fixed_size_buffer_pool.hpp>
#include <wise.kernel/net/buffer/multiple_size_buffer_pool.hpp>
#include <wise.kernel/net/buffer/resize_buffer.hpp>
#include <wise.kernel/net/buffer/segment_buffer.hpp>

#pragma warning(disable : 6319) // intellisense CHECK/REQUIRE 오류로 판단

using namespace wise::kernel;

TEST_CASE("fixed_size_buffer_pool", "net")
{
	SECTION("usage")
	{
		fixed_size_buffer_pool fsb(128);

		CHECK(fsb.get_length() == 128);

		auto bp = fsb.alloc();
		CHECK(bp->get_pool() == &fsb);
		CHECK(bp->is_allocated());
		CHECK(fsb.get_stat().alloc_count == 1);
		CHECK(fsb.get_stat().total_alloc_count == 1);

		fsb.release(bp);
		CHECK(bp->is_released());
		CHECK(fsb.get_stat().alloc_count == 0);
		CHECK(fsb.get_stat().total_release_count == 1);
		CHECK(fsb.get_stat().total_alloc_count == 1);
	}

	SECTION("coverage")
	{
		SECTION("alloc from pool")
		{
			fixed_size_buffer_pool fsb(64);

			auto bp1 = fsb.alloc();
			fsb.release(bp1);

			// alloc from pool
			for (int i = 0; i < 100; ++i)
			{
				auto bp2 = fsb.alloc();
				REQUIRE(bp2->get_hit_count() == i+1);
				fsb.release(bp2);
			}
		}

		SECTION("release nullptr")
		{
			fixed_size_buffer_pool fsb(64);

			auto bp = fixed_size_buffer_pool::buffer::ptr();
			CHECK_THROWS(fsb.release(bp));
		}
	}
}

TEST_CASE("segment", "net")
{
	SECTION("usage")
	{
		segment<32> s1;
		std::string d("0123456789");

		{
			std::size_t added = s1.append(d.c_str(), d.length());
			CHECK(added == d.length());
			CHECK(s1.size() == added);
		}
		{
			std::size_t added = s1.append(d.c_str(), d.length());
			CHECK(added == d.length());
			CHECK(s1.size() == added * 2);
		}
		{
			std::size_t added = s1.append(d.c_str(), d.length());
			CHECK(added == d.length());
			CHECK(s1.size() == added * 3);
		}
		{
			std::size_t added = s1.append(d.c_str(), d.length());
			CHECK(added == 2);
			CHECK(s1.size() == 32);
		}
	}

	SECTION("coverage")
	{
		segment<10> s1;
		std::string d("0123456789");

		CHECK(s1.append(d.c_str(), 0) == 0);
		CHECK(s1.append((uint8_t*)nullptr, 0) == 0);
		CHECK(s1.append((uint8_t*)nullptr, 5) == 0);
	}
}

TEST_CASE("multiple_size_buffer_pool", "net")
{
	SECTION("usage")
	{
		multiple_size_buffer_pool pool;

		auto bp = pool.alloc(10 * 1024);

		CHECK(bp->capacity() == 16 * 1024);
		CHECK(bp->is_allocated());

		pool.release(bp);
		pool.dump_stat();
	}
	
	SECTION("coverage")
	{
		SECTION("alloc out of pool")
		{
			multiple_size_buffer_pool pool;
			
			auto bp = pool.alloc(pool.get_max_size() * 2);
			CHECK(bp->get_pool() == nullptr);
			pool.release(bp);
			bp.reset();

			mem_tracker::inst().report();
		}

		SECTION("release nullptr")
		{
			multiple_size_buffer_pool pool;

			CHECK_THROWS(pool.release(multiple_size_buffer_pool::buffer::ptr()));
		}
	}
}

TEST_CASE("resize_buffer", "net")
{
	SECTION("usage")
	{
		resize_buffer rb;

		std::string d("01234567");

		for (int i = 0; i < 3*1024; ++i)
		{
			rb.append(d.c_str(), d.length());
		}

		CHECK(rb.size() == 3 * 1024 * d.length());
		CHECK(rb.capacity() > rb.size());
		CHECK(rb.at(15) == '7');

		std::array<char, 8> da;
		rb.read((uint8_t*)&da[0], 8);
		CHECK(da[0] == '0');

		CHECK(rb.data()[1] == '1');

		rb.pop_front(3);
		CHECK(rb.data()[0] == '3');

		auto iter = rb.begin();
		CHECK(*iter == '3');
		++iter; 
		CHECK(*iter == '4');

		auto citer = rb.cbegin();
		CHECK(*citer == '3');
		++citer;
		CHECK(*citer == '4');
	}
}

TEST_CASE("segment_buffer", "net")
{
	SECTION("usage")
	{
		segment_buffer<128> sb;

		std::string d("01234567");

		for (int i = 0; i < 1024; ++i)
		{
			sb.append(d.c_str(), d.length());
		}

		CHECK(sb.size() == 1024 * d.length());
		auto size = sb.size();

		auto segs = sb.transfer();

		CHECK(sb.size() == 0);
		CHECK(segs.size() == size / 128);

		sb.release(segs);
	}
}
