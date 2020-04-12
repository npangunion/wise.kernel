#include <pch.hpp>
#include <wise.kernel/net/tcp/tcp_node.hpp>

namespace wise {
namespace kernel {

tcp_node::tcp_node(const config& _config)
	: node(_config)
	, mutex_()
	, config_(_config)
	, seq_(1, UINT16_MAX)
	, acceptors_()
	, connectors_()
	, protocols_()
{
}

tcp_node::~tcp_node()
{
}

node::result tcp_node::listen(const std::string& addr, channel::ptr ch)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	auto id = seq_.next();
	auto ptr = wise_shared<tcp_acceptor>(this, id, addr, ch);

	acceptors_[id] = ptr; // 실패한 acceptor도 보관

	return ptr->listen();
}

node::result tcp_node::connect(const std::string& addr, channel::ptr ch)
{
	tcp_connector::ptr ptr;

	// unqiue lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		auto id = seq_.next();
		ptr = wise_shared<tcp_connector>(this, id, addr, ch);

		// connect 호출하면 connector를 넣기 전에 응답이 오는 경우가 있어 
		// 먼저 맵에 추가해야 한다. 
		connectors_[id] = ptr;
	}

	return ptr->connect();
}

tcp_protocol::ptr tcp_node::get(protocol::id_t id)
{
	std::shared_lock<std::shared_mutex> lock(mutex_);
	auto ptr = protocols_.get(id);
	return std::static_pointer_cast<tcp_protocol>(ptr);
}

node::result tcp_node::on_start()
{
	return result(true, reason::success);
}

void tcp_node::on_finish()
{
	std::size_t pcnt = 0;

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		protocols_.each(
			[](protocols::ptr sp) {
				auto tsp = std::static_pointer_cast<tcp_protocol>(sp);
				tsp->disconnect();
			}
		);

		pcnt = protocols_.get_used_count();
	}

	while (pcnt > 0)
	{
		sleep(5);

		// shared lock
		{
			std::shared_lock<std::shared_mutex> lock(mutex_);
			pcnt = protocols_.get_used_count();
		}
	}
}

void tcp_node::on_accepted(key_t k, tcp::socket&& soc, channel::ptr ch)
{
	// shared lock
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		auto iter = acceptors_.find(k);

		if (iter == acceptors_.end())
		{
			soc.close();
			return;
		}
	}

	on_new_socket(std::move(soc), ch, true);
}

void tcp_node::on_accept_failed(key_t k, const error_code& ec)
{
	WISE_UNUSED(ec);

	tcp_acceptor::ptr apt;

	// shared lock
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		auto iter = acceptors_.find(k);
		if (iter != acceptors_.end())
		{
			apt = iter->second;
		}
	}

	if (apt)
	{
		WISE_ERROR("failed to accept on protocol. addr:{0}", apt->get_addr().get_raw());
	}
}

void tcp_node::on_connected(key_t k, tcp::socket&& soc, channel::ptr ch)
{
	// shared lock
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter == connectors_.end())
		{
			soc.close();
			return;
		}
	}

	on_new_socket(std::move(soc), ch, false);

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		seq_.release(k);
		connectors_.erase(k);
	}
}

void tcp_node::on_connect_failed(key_t k, const error_code& ec)
{
	tcp_connector::ptr cnt;

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter != connectors_.end())
		{
			cnt = iter->second;
			notify_connect_failed(cnt->get_addr().get_raw(), ec);
		}

		seq_.release(k);
		connectors_.erase(k);
	}

	if (cnt)
	{
		WISE_ERROR("failed to connect on protocol. addr:{0}", cnt->get_addr().get_raw());
	}
}

void tcp_node::on_error(protocol::ptr p, const error_code& ec)
{
	WISE_DEBUG("{} remove on error. sid: 0x{:x}", __FUNCTION__, p->get_id());

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		protocols_.del(p->get_id());
	}

	notify_disconnect(std::static_pointer_cast<tcp_protocol>(p), ec);
}

void tcp_node::on_new_socket(
	tcp::socket&& soc, 
	channel::ptr ch,
	bool accepted)
{
	protocol::ptr proto = create_protocol(std::move(soc), accepted);
	
	// unique lock 
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		auto id = protocols_.add(proto);
		proto->set_id(id);
		proto->bind(ch);
	}

	/// 락 풀고 초기화 및 수신 시작
	/**
	 * request_recv()를 하면 바로 응답이 오는 경우가 있다.
	 * 따라서, 호출 전에 관리 기능에 넣어줘야 한다.
	 * begin() 내 초기화 함수들이 느려 락 밖에서 호출한다.
	 */
	auto tp = std::static_pointer_cast<tcp_protocol>(proto);
	tp->begin();

	WISE_INFO(
		"new protocol. id:{:x} local:{} remote:{}", 
		tp->get_id(), 
		tp->get_local_addr(), 
		tp->get_remote_addr());

	// tp->begin() setup address, which are used at following.
	if (accepted)
	{
		notify_accepted(tp);
	}
	else
	{
		notify_connected(tp);
	}
}

} // kernel
} // wise
