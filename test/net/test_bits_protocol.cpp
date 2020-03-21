#include <pch.hpp>
#include <catch.hpp>
#include <wise.kernel/net/protocol/bits/bits_protocol.hpp>
#include <wise.kernel/net/protocol/bits/bits_factory.hpp>
#include <wise.kernel/net/protocol/bits/bits_node.hpp>

using namespace wise::kernel;

// 주의: 여기에 있는 코드는 테스트 용도로 사용법에 대한 가이드가 아님. 
// - service 단에서 처리 방법을 쉽게 만들 예정 
// - 여기는 성능, 안정성, 정확성 테스트 위주

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
		// 개발 시 검증용 설정. 암호화 테스트 등을 위해 사용
		// 네트워크 전송 없이 프로토콜 테스트.

		SECTION("basic flow")
		{
			// session::post()를 사용하여 통지 

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

			// 위와 같이 단위 테스트가 가능한 구조를 미리 갖추는 건 중요하다. 

			network::unsubscribe(sid);
		}

		SECTION("multicast")
		{
			// 미리 serialize 하고 동일 데이터로 여러 연결에 전송

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

			// 기존 멀티캐스트 인터페이스는 암호화와 
			// IO 쓰레드 위주의 송신 처리를 위해 제거.

			zp->send(pkt);

			auto pp = std::static_pointer_cast<bits_test_message>(mp);

			CHECK(pp);
			CHECK(pp->name == "CZenHello");
			CHECK(pp->id == 33);

			network::unsubscribe(sid);
		}

		SECTION("encryption")
		{
			// 미리 serialize 하고 동일 데이터로 여러 연결에 전송

			// factory 수준에서 미리 맞춘다. 

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
				// 별도로 테스트 완료
			}

			SECTION("very small resize buffer")
			{
				// resize_buffer의 초기 크기를 16으로 하고 전체 테스트 진행 
				// 성능 차이가 거의 없다. 풀이라 그런가? 
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

		// TODO: session::ref의 해제 관련 처리

		tester.clear();

		wise::network::inst().finish();

		CHECK(bits_packet::alloc_ == bits_packet::dealloc_);

		// 10만. 1초. 
		// - 괜찮은 정도 
		// - 전에 봤던 수치와 비슷 

		// 10만. 5KB. 2초
		// - 초당 250 메가 바이트
		// - 단일 연결 
		// - 초당 2Gbps로 괜찮은 것 같다. 
		//   - send_op / recv_op가 CPU의 대부분을 차지
	}

	//
	// 완전한 테스트는 부하 테스트 도구를 만들고 진행 
	// IDL C++ 코드 생성까지 먼저 진행한 후 진행 
	// 
}