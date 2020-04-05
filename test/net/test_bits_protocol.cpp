#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>
#include <wise.kernel/net/protocol/bits/bits_packets.hpp>
#include <wise.kernel/util/util.hpp>

using namespace wise::kernel;

// 주의: 여기에 있는 코드는 테스트 용도로 사용법에 대한 가이드가 아님. 
// - service 단에서 처리 방법을 쉽게 만들 예정 
// - 여기는 성능, 안정성, 정확성 테스트 위주

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

		// 많이 보내면 빠르게 진행한다.
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

		// 개발 시 검증용 설정. 암호화 테스트 등을 위해 사용
		// 네트워크 전송 없이 프로토콜 테스트.

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

			bp->bind(ch1); // 동일 채널 사용

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
			// 미리 serialize 하고 동일 데이터로 여러 연결에 전송
			// factory 수준에서 미리 맞춘다. 

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

			bp->bind(ch1); // 동일 채널 사용


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
			// 1백 2십만. 1초 
			// - 순서대로 처리 
			// - 패킷 매번 생성 
			//

			//
			// 인라인 최적화 일부. 952ms. 977ms. 956ms. 954ms 
			// 
		}

		SECTION("edge cases")
		{
			SECTION("divided recv")
			{
				// 나눠서 받을 경우 문제 없는 지 확인
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
				bp->bind(ch1); // 동일 채널 사용

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

		// 에코는 빠른 처리가 되지 않는다. 성능 측정에 적절한 방법은 아니다. 
		// 대신 어느 정도의 성능을 갖는 지 대략 추정할 수 있다.
		// 릴리스. 1백만, 2초. 랩탑. 괜찮은 편이다.
		// 릴리스. 1백만, 1초. i7 pc.
	}
}