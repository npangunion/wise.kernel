#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>

using namespace wise::kernel;

// ����: ���⿡ �ִ� �ڵ�� �׽�Ʈ �뵵�� ������ ���� ���̵尡 �ƴ�. 
// - service �ܿ��� ó�� ����� ���� ���� ���� 
// - ����� ����, ������, ��Ȯ�� �׽�Ʈ ����

namespace
{

struct bits_test_message : public bits_packet
{
	bits_test_message()
		: bits_packet(topic(1, 1, 1))
	{
	}
	
	~bits_test_message()
	{
	}

	bool pack( bits_packer& packer) override 
	{
		packer.pack(name);
		packer.pack(id);

		return packer.is_valid();
	}

	bool unpack( bits_packer& packer) override
	{
		packer.unpack(name);
		packer.unpack(id);

		return packer.is_valid();
	}

	std::string name;
	uint32_t	id;
};

class echo_tester
{
public: 
	echo_tester()
	{
		// factory
		bits_factory::inst().add(
			topic(1, 1, 1), 
			[]() 
			{ 
				return wise_shared<bits_test_message>();
			});

		// sub to messages
		network::subscribe(
			topic(sys::category::sys, sys::group::net, sys::type::session_ready),
			[this](message::ptr m) 
			{
				on_ready(m);
			});

		network::subscribe(
			topic(1, 1, 1), 
			[this](message::ptr m) { on_echo(m); }
		);

		network::subscribe(
			topic(sys::category::sys, sys::group::net, sys::type::session_closed), 
			[this](message::ptr m)
		{
			on_closed(m);
		});
	}

	int get_seq() const
	{
		return seq_;
	}

	void clear()
	{
		s_.reset();
	}

private: 
	void on_ready(message::ptr m)
	{
		auto zp = std::static_pointer_cast<sys_session_ready>(m);

		network& net = network::inst();
		auto s = net.acquire(session::id(zp->sid));

		auto em = wise_shared<bits_test_message>();

		for (int i = 0; i < 512; ++i)
		{
			em->name.append("AZenHello");
		}

		em->id = seq_++;

		s.send(em);
	}

	void on_echo(message::ptr m)
	{
		auto rm = std::static_pointer_cast<bits_test_message>(m);
		auto em = wise_shared<bits_test_message>();

		em->name = "AZenHelloResp";
		em->id = seq_++;

		auto sref = network::inst().acquire(session::id(rm->sid));
		sref.send(em);
	}

	void on_closed(message::ptr m)
	{

	}

private: 
	int seq_ = 0;
	session_ref s_;
};

} // noname


TEST_CASE("bits protocol")
{
	SECTION("loopback test")
	{
		// ���� �� ������ ����. ��ȣȭ �׽�Ʈ ���� ���� ���
		// ��Ʈ��ũ ���� ���� �������� �׽�Ʈ.

		SECTION("basic flow")
		{
			// session::post()�� ����Ͽ� ���� 

			bits_factory::inst().add(
				topic(1, 1, 1),
				[]() { return wise_shared<bits_test_message>(); }
			);

			bits_protocol::cfg.enable_loopback = true;

			message::ptr mp;

			auto sid = network::subscribe(
				topic(1, 1, 1),
				[&mp](message::ptr m) {
					WISE_INFO("message recv. topic: 0x{:x}",
						m->get_topic().get_key());
					mp = m;
				}
			);

			auto zp = wise_shared<bits_protocol>();

			auto pkt = wise_shared<bits_test_message>();

			pkt->name = "BZenHello";
			pkt->id = 33;

			zp->send(pkt);

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp);
			CHECK(pp->name == "BZenHello");
			CHECK(pp->id == 33);

			// ���� ���� ���� �׽�Ʈ�� ������ ������ �̸� ���ߴ� �� �߿��ϴ�. 

			network::unsubscribe(sid);
		}

		SECTION("multicast")
		{
			// �̸� serialize �ϰ� ���� �����ͷ� ���� ���ῡ ����

			bits_factory::inst().add(
				topic(1, 1, 1),
				[]() { return wise_shared<bits_test_message>(); }
			);

			bits_protocol::cfg.enable_loopback = true;

			message::ptr mp;

			auto sid = network::subscribe(
				topic(1, 1, 1),
				[&mp](message::ptr m) {
					WISE_INFO("message recv. topic: 0x{:x}", m->get_topic().get_key());
					mp = m;
				}
			);

			auto zp = wise_shared<bits_protocol>();

			auto pkt = wise_shared<bits_test_message>();

			pkt->name = "CZenHello";
			pkt->id = 33;

			// ���� ��Ƽĳ��Ʈ �������̽��� ��ȣȭ�� 
			// IO ������ ������ �۽� ó���� ���� ����.

			zp->send(pkt);

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp);
			CHECK(pp->name == "CZenHello");
			CHECK(pp->id == 33);

			network::unsubscribe(sid);
		}

		SECTION("encryption")
		{
			// �̸� serialize �ϰ� ���� �����ͷ� ���� ���ῡ ����

			// factory ���ؿ��� �̸� �����. 

			bits_factory::inst().add(
				topic(1, 1, 1),
				[]() {
					auto mp = wise_shared<bits_test_message>();
					mp->enable_checksum = true;
					mp->enable_cipher = true;
					mp->enable_sequence = true;

					return mp;
				}
			);

			bits_protocol::cfg.enable_loopback = true;

			bits_protocol::cfg.enable_cipher = true;
			bits_protocol::cfg.enable_sequence = true;
			bits_protocol::cfg.enable_checksum = true;

			message::ptr mp;

			auto sid = network::subscribe(
				topic(1, 1, 1),
				[&mp](message::ptr m) {
					WISE_INFO("message recv. topic: 0x{:x}", m->get_topic().get_key());
					mp = m;
				}
			);

			auto zp = wise_shared<bits_protocol>();

			auto sp = bits_factory::inst().create(topic(1, 1, 1));
			auto pkt = std::static_pointer_cast<bits_test_message>(sp);

			pkt->name = "Hello w/ encryption";
			pkt->id = 33;

			// pack and send bytes

			zp->send(pkt);

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp->name == "Hello w/ encryption");
			CHECK(pp->id == 33);

			network::unsubscribe(sid);

			bits_protocol::cfg.enable_cipher = false;
			bits_protocol::cfg.enable_sequence = false;
			bits_protocol::cfg.enable_checksum = false;
		}

		SECTION("loopback performance")
		{

			bits_factory::inst().add(
				topic(1, 1, 1),
				[]() { return wise_shared<bits_test_message>(); }
			);

			bits_protocol::cfg.enable_loopback = true;

			message::ptr mp;

			auto zp = wise_shared<bits_protocol>();

			const int test_count = 1; // 1200000;

			fine_tick tick;

			for (int i = 0; i < test_count; ++i)
			{
				auto pkt = wise_shared<bits_test_message>();

				pkt->name = "Hello";
				pkt->id = 33;

				zp->send(pkt);
			}

			WISE_INFO("loopback performance. elapsed: {}", tick.elapsed());


			// 
			// 1�� 2�ʸ�. 1�� 
			// - ������� ó�� 
			// - ��Ŷ �Ź� ���� 
			//

			//
			// �ζ��� ����ȭ �Ϻ�. 952ms. 977ms. 956ms. 954ms 
			// 
		}

		SECTION("edge cases")
		{
			SECTION("divided recv")
			{
				// ������ ���� ��� ���� ���� �� Ȯ��
				bits_factory::inst().add(
					topic(1, 1, 1),
					[]() { return wise_shared<bits_test_message>(); }
				);

				bits_protocol::cfg.enable_loopback = true;

				message::ptr mp;

				auto sid = network::subscribe(
					topic(1, 1, 1),
					[&mp](message::ptr m) {
						WISE_INFO("message recv. topic: 0x{:x}", m->get_topic().get_key());
						mp = m;
					}
				);

				auto zp = wise_shared<bits_protocol>();

				auto pkt = wise_shared<bits_test_message>();

				pkt->name = "DZenHello";
				pkt->id = 33;

				resize_buffer buf;

				bits_protocol::pack(pkt, buf);

				zp->on_recv_to_test(buf.data(), buf.size() - 8);
				zp->on_recv_to_test(buf.data() + buf.size() - 8, 8);

				auto pp = std::static_pointer_cast<bits_test_message>(mp);

				CHECK(pp);
				CHECK(pp->name == "DZenHello");
				CHECK(pp->id == 33);

				network::unsubscribe(sid);
			}

			SECTION("very long string")
			{
				// ������ �׽�Ʈ �Ϸ�
			}

			SECTION("very small resize buffer")
			{
				// resize_buffer�� �ʱ� ũ�⸦ 16���� �ϰ� ��ü �׽�Ʈ ���� 
				// ���� ���̰� ���� ����. Ǯ�̶� �׷���? 
			}
		}
	}

	SECTION("echo performance")
	{
		bits_protocol::cfg.enable_loopback = false;

		echo_tester tester;  // subscription

		const int test_count = 10;

		network::inst().start();

		network::inst().listen("0.0.0.0:7777", "bits");

		network::inst().connect("127.0.0.1:7777", "bits");

		fine_tick tick;

		while (true)
		{
			wise::sleep(10);

			auto seq = tester.get_seq();

			if (seq >= test_count)
			{
				break;
			}
		}

		WISE_INFO("echo test. elapsed: {}, count: {}", tick.elapsed(), test_count);

		// TODO: session::ref�� ���� ���� ó��

		tester.clear();

		wise::network::inst().finish();

		CHECK(bits_packet::alloc_ == bits_packet::dealloc_);

		// 10��. 1��. 
		// - ������ ���� 
		// - ���� �ô� ��ġ�� ��� 

		// 10��. 5KB. 2��
		// - �ʴ� 250 �ް� ����Ʈ
		// - ���� ���� 
		// - �ʴ� 2Gbps�� ������ �� ����. 
		//   - send_op / recv_op�� CPU�� ��κ��� ����
	}

	//
	// ������ �׽�Ʈ�� ���� �׽�Ʈ ������ ����� ���� 
	// IDL C++ �ڵ� �������� ���� ������ �� ���� 
	// 
}