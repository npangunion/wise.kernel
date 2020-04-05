#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>
#include <wise.kernel/util/util.hpp>

using namespace wise::kernel;

// ����: ���⿡ �ִ� �ڵ�� �׽�Ʈ �뵵�� ������ ���� ���̵尡 �ƴ�. 
// - service �ܿ��� ó�� ����� ���� ���� ���� 
// - ����� ����, ������, ��Ȯ�� �׽�Ʈ ����

namespace
{

struct bits_test_message : public bits_packet
{
	bits_test_message()
		: bits_packet(topic(2, 1, 1))
		, name()
		, id(0)
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
	echo_tester(bits_node& bn)
		: bn_(bn)
	{
		channel::config cfg;

		cfg.loop_post_limit = 1000;

		ch_ = wise_shared<channel>("tester", cfg);
		bn_.bind(ch_);

		// factory
		bits_factory::inst().add(
			topic(2, 1, 1), 
			[]() 
			{ 
				return wise_shared<bits_test_message>();
			});

		// sub to messages
		ch_->subscribe(
			topic(bits_topic::connected()),
			[this](message::ptr m) 
			{
				on_connected(m);
			});

		ch_->subscribe(
			topic(bits_topic::accepted()),
			[this](message::ptr m)
			{
				on_accepted(m);
			});


		ch_->subscribe(
			topic(2, 1, 1), 
			[this](message::ptr m) { on_echo(m); }
		);

		ch_->subscribe(
			topic(bits_topic::disconnected()),
			[this](message::ptr m) 
			{
				on_closed(m); 
			});
	}

	void execute()
	{
		ch_->execute();
	}

	int get_seq() const
	{
		return seq_;
	}

	bool is_busy() const
	{
		return ch_->get_queue_size() > 0;
	}

	void clear()
	{
		ch_->clear();
	}

private: 
	void on_connected(message::ptr m)
	{
		auto bp = std::static_pointer_cast<bits_syn_connected>(m);

		bp->get_protocol()->bind(ch_);

		// ���� ������ ������ �����Ѵ�.
		for (int i = 0; i < 256; ++i)
		{
			auto em = wise_shared<bits_test_message>();
			em->name.append("ABitsHello");
			em->id = seq_++;

			bp->send(em);
		}
	}

	void on_accepted(message::ptr m)
	{
		auto bp = std::static_pointer_cast<bits_syn_accepted>(m);

		bp->get_protocol()->bind(ch_);

		for (int i = 0; i < 256; ++i)
		{
			auto em = wise_shared<bits_test_message>();
			em->name.append("ABitsHello");
			em->id = seq_++;

			bp->send(em);
		}
	}

	void on_echo(message::ptr m)
	{
		auto rm = std::static_pointer_cast<bits_test_message>(m);

		auto em = wise_shared<bits_test_message>();
		em->name = "ABitsHelloResp";
		em->id = seq_++;

		rm->send(em);
	}

	void on_closed(message::ptr m)
	{
	}

private: 
	bits_node& bn_;
	std::atomic<int> seq_ = 0;
	channel::ptr ch_;
};

} // noname


TEST_CASE("bits protocol")
{
	SECTION("loopback test")
	{
		bits_node::config cfg;
		bits_node bn(cfg);

		// ���� �� ������ ����. ��ȣȭ �׽�Ʈ ���� ���� ���
		// ��Ʈ��ũ ���� ���� �������� �׽�Ʈ.

		SECTION("basic flow")
		{
			bits_factory::inst().add(
				topic(2, 1, 1),
				[]() { return wise_shared<bits_test_message>(); }
			);

			bits_protocol::cfg.enable_loopback = true;

			channel::ptr ch1 = wise_shared<channel>("c1");
			message::ptr mp;

			auto sid = ch1->subscribe(
				topic(2, 1, 1),
				[&mp](message::ptr m) {
					WISE_INFO("message recv. topic: 0x{:x}",
						m->get_topic().get_key());
					mp = m;
				}
			);


			tcp::socket sock(bn.ios());
			tcp_node* tn = static_cast<tcp_node*>(&bn);

			auto bp = wise_shared<bits_protocol>(tn, sock, true);
			auto pkt = wise_shared<bits_test_message>();

			bp->bind(ch1); // ���� ä�� ���

			pkt->name = "BBitsHello";
			pkt->id = 33;
			bp->send(pkt);

			ch1->execute();

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp);
			CHECK(pp->name == "BBitsHello");
			CHECK(pp->id == 33);

			ch1->unsubscribe(sid);
			ch1->clear();
		}

		SECTION("encryption")
		{
			// �̸� serialize �ϰ� ���� �����ͷ� ���� ���ῡ ����
			// factory ���ؿ��� �̸� �����. 

			bits_factory::inst().add(
				topic(2, 1, 1),
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

			channel::ptr ch1 = wise_shared<channel>("c1");

			auto sid = ch1->subscribe(
				topic(2, 1, 1),
				[&mp](message::ptr m) {
					WISE_INFO("message recv. topic: 0x{:x}", m->get_topic().get_key());
					mp = m;
				}
			);

			tcp::socket sock(bn.ios());
			tcp_node* tn = static_cast<tcp_node*>(&bn);

			auto bp = wise_shared<bits_protocol>(tn, sock, true);

			bp->bind(ch1); // ���� ä�� ���


			auto sp = bits_factory::inst().create(topic(2, 1, 1));
			auto pkt = std::static_pointer_cast<bits_test_message>(sp);

			pkt->name = "Hello w/ encryption";
			pkt->id = 33;

			// pack and send bytes

			bp->send(pkt);

			ch1->execute();

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp->name == "Hello w/ encryption");
			CHECK(pp->id == 33);

			ch1->unsubscribe(sid);

			bits_protocol::cfg.enable_cipher = false;
			bits_protocol::cfg.enable_sequence = false;
			bits_protocol::cfg.enable_checksum = false;
		}

		SECTION("loopback performance")
		{

			bits_factory::inst().add(
				topic(2, 1, 1),
				[]() { return wise_shared<bits_test_message>(); }
			);

			bits_protocol::cfg.enable_loopback = true;

			message::ptr mp;

			tcp::socket sock(bn.ios());
			tcp_node* tn = static_cast<tcp_node*>(&bn);

			auto bp = wise_shared<bits_protocol>(tn, sock, true);

			const int test_count = 12000;

			fine_tick tick;

			auto pkt = wise_shared<bits_test_message>();
			pkt->name = "Hello";
			pkt->id = 33;

			for (int i = 0; i < test_count; ++i)
			{
				bp->send(pkt);
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
					topic(2, 1, 1),
					[]() { return wise_shared<bits_test_message>(); }
				);

				bits_protocol::cfg.enable_loopback = true;

				message::ptr mp;
				channel::ptr ch1 = wise_shared<channel>("c1");

				auto sid = ch1->subscribe(
					topic(2, 1, 1),
					[&mp](message::ptr m) {
						WISE_INFO("message recv. topic: 0x{:x}", m->get_topic().get_key());
						mp = m;
					}
				);

				tcp::socket sock(bn.ios());
				tcp_node* tn = static_cast<tcp_node*>(&bn);

				auto bp = wise_shared<bits_protocol>(tn, sock, true);
				bp->bind(ch1); // ���� ä�� ���

				auto pkt = wise_shared<bits_test_message>();
				pkt->name = "DBitsHello";
				pkt->id = 33;

				resize_buffer buf;

				bp->pack(pkt, buf);

				bp->on_recv_to_test(buf.data(), buf.size() - 8);
				bp->on_recv_to_test(buf.data() + buf.size() - 8, 8);

				ch1->execute();

				auto pp = std::static_pointer_cast<bits_test_message>(mp);

				CHECK(pp);
				CHECK(pp->name == "DBitsHello");
				CHECK(pp->id == 33);

				ch1->unsubscribe(sid);
			}
		}
	}

	SECTION("echo performance")
	{
		bits_protocol::cfg.enable_loopback = false;

		mem_tracker::inst().disable();

		tcp_node::config cfg;
		bits_node bn(cfg);

		echo_tester tester(bn);  // subscription

		const int test_count = 100;

		bn.start();

		bn.listen("0.0.0.0:7777");
		bn.connect("127.0.0.1:7777");

		fine_tick tick;

		while (true)
		{
			auto seq = tester.get_seq();

			if (seq >= test_count)
			{
				break;
			}

			if (!tester.is_busy())
			{
				// sleep(1);  
			}

			tester.execute();
		}

		WISE_INFO("echo test. elapsed: {}, count: {}", tick.elapsed(), tester.get_seq());

		tester.clear();

		bn.finish();

		mem_tracker::inst().enable();

		// ���ڴ� ���� ó���� ���� �ʴ´�. ���� ������ ������ ����� �ƴϴ�. 
		// ��� ��� ������ ������ ���� �� �뷫 ������ �� �ִ�.
		// ������. 1�鸸, 2��. ��ž. ������ ���̴�.
		// ������. 1�鸸, 1��. i7 pc.
	}
}