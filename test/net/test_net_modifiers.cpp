#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/modifier/checksum.hpp>
#include <wise.kernel/net/modifier/cipher.hpp>
#include <wise.kernel/net/modifier/sequencer.hpp>

#pragma warning(disable : 6319) // intellisense CHECK/REQUIRE 오류로 판단

using namespace wise::kernel;

TEST_CASE("sequencer", "net")
{
	SECTION("usage")
	{
		sequencer seq;
		resize_buffer rb;

		std::size_t len = 0;

		rb.append(&len, 4);
		rb.append("hello", 5);

		auto send_size = rb.size();

		seq.on_send(rb, 0, rb.size());

		CHECK(rb.size() == send_size + 1);
		CHECK(rb.at(rb.size() - 1) == 1);

		std::size_t nl = 0;
		auto rc = seq.on_recv(rb, 0, rb.size(), nl);

		CHECK(rc);
		CHECK(nl == send_size);
	}
}

TEST_CASE("checksum", "net")
{
	SECTION("usage")
	{
		checksum cs(4); 
		resize_buffer rb;

		std::size_t len = 0;

		rb.append(&len, 4);
		rb.append("hello", 5);

		auto send_size = rb.size();

		cs.on_send(rb, 0, rb.size());

		CHECK(rb.size() == send_size + 4);

		std::size_t nl = 0;
		auto rc = cs.on_recv(rb, 0, rb.size(), nl);

		CHECK(rc);
		CHECK(nl == send_size);
	}
}

TEST_CASE("cipher", "net")
{
	SECTION("usage")
	{
		cipher cs(4);
		resize_buffer rb;

		for (int i = 0; i < 1; ++i)
		{
			std::size_t len = 0;

			rb.append(&len, 4);
			rb.append("hello", 5);

			auto send_size = rb.size();

			cs.on_send(rb, 0, rb.size());

			std::size_t nl = 0;
			auto rc = cs.on_recv(rb, 0, rb.size(), nl);

			CHECK(rc);
			CHECK(nl == send_size);

			std::string rs((const char*)rb.data() + 4, 5);
			CHECK(rs == "hello");
		}
	}
}