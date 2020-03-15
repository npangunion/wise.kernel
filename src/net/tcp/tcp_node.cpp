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

node::result tcp_node::listen(const std::string& proto, const std::string& addr, channel::ptr ch)
{
	std::unique_lock<std::shared_mutex> lock(mutex_);

	auto id = seq_.next();
	auto ptr = wise_shared<tcp_acceptor>(this, id, proto, addr, ch);
	auto rc = ptr->listen();

	if (rc)
	{
		acceptors_[id] = ptr;
	}

	return rc;
}

node::result tcp_node::connect(const std::string& proto, const std::string& addr, channel::ptr ch)
{
	tcp_connector::ptr ptr;

	// unqiue lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		auto id = seq_.next();
		ptr = wise_shared<tcp_connector>(this, id, proto, addr, ch);

		// connect ȣ���ϸ� connector�� �ֱ� ���� ������ ���� ��찡 �־� 
		// ���� �ʿ� �߰��ؾ� �Ѵ�. 
		connectors_[id] = ptr;
	}

	(void)ptr->connect();

	return result(true, reason::success);
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
	std::unique_lock<std::shared_mutex> lock(mutex_);

	protocols_.each(
		[](protocols::ptr sp) {
			auto tsp = std::static_pointer_cast<tcp_protocol>(sp);
			tsp->disconnect();
		}
	);

	protocols_.clear();
}

void tcp_node::on_accepted(key_t k, tcp::socket&& soc)
{
	std::string protocol;
	channel::ptr chan;

	// shared lock
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		auto iter = acceptors_.find(k);

		if (iter == acceptors_.end())
		{
			soc.close();
			return;
		}

		protocol = iter->second->get_protocol();
		chan = iter->second->get_channel();
	}

	on_new_socket(protocol, chan, std::move(soc), true);
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
		WISE_ERROR(
			"failed to accept on protocol:{0}, addr:{1}",
			apt->get_protocol(),
			apt->get_addr().get_raw()
		);
	}
}

void tcp_node::on_connected(key_t k, tcp::socket&& soc)
{
	std::string protocol;
	channel::ptr chan;

	// shared lock
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter == connectors_.end())
		{
			soc.close();
			return;
		}

		protocol = iter->second->get_protocol();
		chan = iter->second->get_channel();
	}

	on_new_socket(protocol, chan, std::move(soc), false);

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		seq_.release(k);
		connectors_.erase(k);
	}
}

void tcp_node::on_connect_failed(key_t k, const error_code& ec)
{
	WISE_UNUSED(ec);

	tcp_connector::ptr cnt;

	// unique lock
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		auto iter = connectors_.find(k);
		if (iter != connectors_.end())
		{
			cnt = iter->second;
		}

		seq_.release(k);
		connectors_.erase(k);
	}

	// TODO: notify
}

void tcp_node::on_new_socket(
	const std::string& protocol, 
	channel::ptr chan, 
	tcp::socket&& soc, 
	bool accepted)
{
	protocol::ptr proto = create_protocol(protocol, std::move(soc), accepted);
	proto->bind(chan);
	
	// unique lock 
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);

		auto id = protocols_.add(proto);
		proto->set_id(id);
	}

	/// �� Ǯ�� �ʱ�ȭ �� ���� ����
	/**
	 * request_recv()�� �ϸ� �ٷ� ������ ���� ��찡 �ִ�.
	 * ����, ȣ�� ���� ���� ��ɿ� �־���� �Ѵ�.
	 * begin() �� �ʱ�ȭ �Լ����� ���� �� �ۿ��� ȣ���Ѵ�.
	 */
	auto tp = std::static_pointer_cast<tcp_protocol>(proto);
	tp->begin();

	// TODO: notify
}

} // kernel
} // wise
