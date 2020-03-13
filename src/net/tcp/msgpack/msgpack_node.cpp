#include <pch.hpp>
#include <wise.kernel/net/detail/network_impl.hpp>
#include <wise.kernel/net/protocol/sys_messages.hpp>
#include <wise.kernel/net/protocol/protocol_factory.hpp>
#include <wise.kernel/net/protocol/zen/zen_protocol.hpp>
#include <wise.kernel/core/exception.hpp>
#include <wise.kernel/core/util.hpp>
#include <wise.kernel/core/memory.hpp>

namespace wise
{


bool network_impl::post_stopped_ = false;

// check_delayed_sub_before_enqueue를 true로 하여 큐에 넣지 않도록 한다
channel network_impl::channel_("global", channel::config{ 2048, 20, 200, false, true });

sub::key_t network_impl::subscribe(
	const topic& topic,
	cond_t cond,
	cb_t cb
)
{
	return channel_.subscribe(topic, cond, cb, sub::mode::immediate);
}

sub::key_t network_impl::subscribe(
	const topic& topic,
	cb_t cb
)
{
	return channel_.subscribe(topic, cb, sub::mode::immediate);
}

bool network_impl::unsubscribe(sub::key_t key)
{
	return channel_.unsubscribe(key);
}

void network_impl::post(packet::ptr m)
{
	if (post_stopped_)
	{
		WISE_WARN("lost packet. topic:{}", topic::get_desc(m->get_topic()));
		return;
	}

	WISE_TRACE("post from network. message:{} session:0x{:x}", m->get_desc(), m->sid);

	channel_.publish_immediate(m);
}

void network_impl::clear_subscription()
{
	channel_.clear();
}

void network_impl::stop_post_to_finish()
{
	post_stopped_ = true;
}

network_impl::network_impl(network& svc)
	: svc_(svc)
	, ios_()
	, id_gen_(1, MAXUINT16)
{
}

network_impl::~network_impl()
{
}

network::result network_impl::listen(const std::string& addr, const std::string& protocol, uintptr_t pkey)
{
	if (!protocol_factory::inst().has(protocol))
	{
		WISE_CRITICAL(
			"protocol {0} not implemented or added", protocol
		);

		return network::result(false, reason::fail_protocol_not_added);
	}

	// add acceptors
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		auto id = id_gen_.next();
		auto ptr = wise_shared<acceptor>(id, protocol, addr, pkey);
		auto rc = ptr->listen();

		if (rc)
		{
			acceptors_[id] = ptr;
			++acceptor_count_;
		}

		return rc;
	}
}

network::result network_impl::connect(const std::string& addr, const std::string& protocol, uintptr_t pkey)
{
	if (!protocol_factory::inst().has(protocol))
	{
		WISE_CRITICAL(
			"protocol {0} not implemented or added", protocol
		);

		return network::result(false, reason::fail_protocol_not_added);
	}

	connector::ptr ptr;

	// unqiue lock
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		auto id = id_gen_.next();
		ptr = wise_shared<connector>(id, protocol, addr, pkey);

		// connect 호출하면 connector를 넣기 전에 응답이 오는 경우가 있어 
		// 먼저 맵에 추가해야 한다. 
		connectors_[id] = ptr;
		++connector_count_;
	}

	(void)ptr->connect();

	return network::result(true, success);
}

session_ref network_impl::acquire(const session::id& id)
{
	WISE_ASSERT(id.get_value() > 0);
	WISE_RETURN_IF(id.get_value() == 0, session_ref());

	std::shared_lock<std::shared_timed_mutex> lock(mutex_);

	WISE_RETURN_IF(sessions_.empty() || id.get_index() > sessions_.size(), session_ref());
	WISE_RETURN_IF(!sessions_[id.get_index()].session, session_ref());
	// 같은 아이디가 아니면 다른 무효한 세션이어야 함
	WISE_RETURN_IF(sessions_[id.get_index()].session->get_id() != id, session_ref());

	auto sp = sessions_[id.get_index()];
	return session_ref(sp.session);
}

bool network_impl::is_running() const
{
	return !stop_;
}

void network_impl::destroy(const session::id& id, const asio::error_code& ec)
{
	WISE_DEBUG( "{} remove on error. sid: 0x{:x}", __FUNCTION__, id.get_value() );

	session::ptr sp2;

	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		auto idx = id.get_index();
		WISE_ASSERT(idx < sessions_.size());

		sp2 = sessions_[idx].session;

		// XXX: 이런 경우가 생기면 안 됨. 디버깅 필요
		WISE_ASSERT(!!sp2);
		WISE_ASSERT(sp2->get_id() == id);

		sessions_[idx].session.reset();

		free_slots_.push_back(idx);

		session_count_--;

		WISE_ASSERT(free_slots_.size() + session_count_ == sessions_.size());
	}

	if (sp2)
	{
		notify_session_closed(sp2, ec);
	}
}

void network_impl::on_accepted(key k, tcp::socket&& soc)
{
	std::string protocol;
	uintptr_t pkey = 0;

	// shared lock
	{
		std::shared_lock<std::shared_timed_mutex> lock(mutex_);

		auto iter = acceptors_.find(k);

		if (iter == acceptors_.end())
		{
			soc.close();
			return;
		}

		protocol = iter->second->get_protocol();
		pkey = iter->second->get_pkey();
	}

	auto sp = on_new_socket(protocol, std::move(soc), true);
	sp->bind_channel(pkey);
	notify_session_ready(sp);
}

void network_impl::on_accept_failed(key k, const asio::error_code& ec)
{
	acceptor::ptr apt;

	// shared lock
	{
		std::shared_lock<std::shared_timed_mutex> lock(mutex_);

		auto iter = acceptors_.find(k);
		if (iter != acceptors_.end())
		{
			apt = iter->second;
		}
	}

	if (apt)
	{
		WISE_ERROR(
			"failed to accept on protocol:{0}, addr:{1}", 
			apt->get_protocol(), 
			apt->get_addr().get_raw()
		);

		auto mp = wise_shared<sys_accept_failed>();
		mp->addr = apt->get_addr().get_raw();
		mp->ec = ec;

		network_impl::post(mp);
	}
}

void network_impl::on_connected(key k, tcp::socket&& soc)
{
	std::string protocol;
	uintptr_t pkey = 0;

	// shared lock
	{
		std::shared_lock<std::shared_timed_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter == connectors_.end())
		{
			soc.close();
			return;
		}

		protocol = iter->second->get_protocol();
		pkey = iter->second->get_pkey();
	}

	auto sp = on_new_socket(protocol, std::move(soc), false);
	sp->bind_channel(pkey);
	notify_session_ready(sp);

	// unique lock
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		id_gen_.release(k);
		connectors_.erase(k);
		--connector_count_;
	}
}

void network_impl::on_connect_failed(key k, const asio::error_code& ec)
{
	connector::ptr cnt;

	// unique lock
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter != connectors_.end())
		{
			cnt = iter->second;
		}

		id_gen_.release(k);
		connectors_.erase(k);
		--connector_count_;
	}

	if (cnt)
	{
		auto mp = wise_shared<sys_connect_failed>();
		mp->addr = cnt->get_addr().get_raw();
		mp->ec = ec;
		mp->pkey = cnt->get_pkey();

		network_impl::post(mp);
	}
}

uint16_t network_impl::get_acceptor_count() const
{
	return acceptor_count_;
}

uint16_t network_impl::get_connector_count() const
{
	return connector_count_;
}

bool network_impl::start()
{
	std::unique_lock<std::shared_timed_mutex> lock(mutex_);

	// allow start when already started
	WISE_RETURN_IF(!stop_, true);

	auto n = network::cfg.concurreny_level;

	if (network::cfg.use_hardware_concurreny)
	{
		n = std::thread::hardware_concurrency();
	}

	WISE_ASSERT(n > 0);
	WISE_ASSERT(stop_);

	stop_ = false;

	threads_.resize(n);

	init_internal_protocols();

	for (int i = 0; i < n; ++i)
	{
		auto thread = std::thread([this]() {this->run();});
		threads_[i].swap(thread);
	}

	WISE_INFO("network started");

	return true;
}

void network_impl::run()
{
	while (!stop_)
	{
		auto n = ios_.run();

		if ( n == 0 ) 
		{
			// 처리 한 요청이 없으면 잠시 쉰다. 
			sleep(1);
		}
	}
}

void network_impl::finish()
{
	// thread safety is required
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);
		WISE_RETURN_IF(stop_);
		stop_ = true;
	}

	// post to all threads
	for (auto& t : threads_)
	{
		WISE_UNUSED(t);
		ios_.stop(); 
	}

	// wait all threads
	for (auto& t : threads_)
	{
		t.join();
	}

	cleanup();

	resize_buffer::clear();

	WISE_INFO("network finished");
}

void network_impl::cleanup()
{
	// 명시적으로 정리하면 안정성 관련 오류를 찾기 쉽다.

	sessions_.clear();
	acceptors_.clear();
	connectors_.clear();
	threads_.clear();
	free_slots_.clear();
	threads_.clear();

	network_impl::clear_subscription();
	protocol_factory::inst().clear();
}

session::ptr network_impl::on_new_socket(const std::string& protocol, tcp::socket&& soc, bool accepted)
{
	session::ptr nsp;

	// unique lock 
	{
		std::unique_lock<std::shared_timed_mutex> lock(mutex_);

		uint16_t slot_idx = slot_idx = get_free_slot();
		uint16_t seq = 0;

		seq = sessions_[slot_idx].seq++;
		seq = (seq == 0 ? sessions_[slot_idx].seq++ : seq);

		WISE_ASSERT(seq > 0);

		nsp = wise_shared<session>(
			session::id(seq, slot_idx),
			std::move(soc),
			accepted,
			protocol
			);

		sessions_[slot_idx].session = nsp;
		session_count_++;

		WISE_ENSURE(sessions_[slot_idx].session);
		WISE_ENSURE(session_count_ > 0);
	}

	/// 락 풀고 초기화 및 수신 시작
	/** 
	 * request_recv()를 하면 바로 응답이 오는 경우가 있다. 
	 * 따라서, 호출 전에 관리 기능에 넣어줘야 한다. 
	 * begin() 내 초기화 함수들이 느려 락 밖에서 호출한다.
	 */
	nsp->begin();

	return nsp;
}

void network_impl::notify_session_ready(session::ptr ss)
{
	WISE_DEBUG(
		"welcome! {} sid: 0x{:x}", 
		ss->is_accepted() ? "accepted" : "connected", ss->get_id().get_value() 
	);

	auto mp = wise_shared<sys_session_ready>();
	mp->sid = ss->get_id().get_value();
	mp->accepted = ss->is_accepted();

	network_impl::post(mp); // send to channel
}

void network_impl::notify_session_closed(session::ptr ss, const asio::error_code& ec)
{
	WISE_DEBUG(
		"network_impl closed. sid: 0x{:x}", ss->get_id().get_value()
	);

	// 소멸 통지와 subscribe_close가 락으로 순서를 잡는다. 
	// 안 그러면 소멸 통지를 못 받는 등록이 발생한다. 
	std::lock_guard<session::lock_type> lock(ss->get_session_lock());

	auto mp = wise_shared<sys_session_closed>();
	mp->sid = ss->get_id().get_value();
	mp->ec = ec;
	mp->accepted = ss->is_accepted();

	// 연결 종료는 링크들에 통지해야 함
	ss->post(mp);
}

void network_impl::init_internal_protocols()
{
	protocol_factory::inst().add(
		"zen", 
		[]() { return wise_shared<zen_protocol>(); }
	);
}

uint16_t network_impl::get_free_slot()
{
	// 외부에서 unique 락 거는 것으로 가정

	WISE_ASSERT(sessions_.size() < UINT16_MAX);

	if (sessions_.size() >= UINT16_MAX)
	{
		WISE_THROW("network_impl slot limit reached");
	}

	if (free_slots_.empty())
	{
		sessions_.push_back({session::ptr(), 0});
		
		return static_cast<uint16_t>(sessions_.size() - 1);
	}

	auto index = free_slots_.front();

	free_slots_.pop_front();

	WISE_ASSERT(index < sessions_.size());

	return index;
}

uint16_t network_impl::get_session_count() const
{
	return session_count_;
}

} // wise
