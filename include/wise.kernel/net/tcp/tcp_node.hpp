#pragma once

#include <wise.kernel/net/tcp/tcp_acceptor.hpp>
#include <wise.kernel/net/tcp/tcp_connector.hpp>
#include <wise.kernel/net/tcp/tcp_protocol.hpp>
#include <wise.kernel/net/util/slot_vector.hpp>
#include <wise.kernel/net/node.hpp>
#include <wise.kernel/net/protocol_factory.hpp>

namespace wise
{
namespace kernel
{

/// abstract base class of tcp nodes
/**
 * �⺻ ��� ����� �����Ͽ� ����Ʈ�� �ۼ����� �� �ִ�.
 * tcp �� �޼��� �帧�� �����ϴ� ���������� �����.
 * @see msgpack_node
 */
class tcp_node : public node
{
public:
	friend class tcp_acceptor;
	friend class tcp_connector;

	struct config : public node::config 
	{
		bool set_tcp_no_delay = true;			/// nagle disable
		bool set_tcp_linger = true;				/// SO_LINGER �ѱ�
		bool set_tcp_reuse_addr = true;			/// ���� ���񽺿����� ���� ���� ����
	};

public:
	tcp_node(const config& _config);

	~tcp_node();

	/// addr���� listen. addr�� ip:port ����. 
	result listen(const std::string& addr);

	/// connect to a addr. addr�� ip:port ����
	result connect(const std::string& addr);

	tcp_protocol::ptr get(protocol::id_t id);

	const config& get_config() const
	{
		return config_;
	}

protected:
	result on_start() override;

	void on_finish() override;

	virtual protocol::ptr create_protocol(
		tcp::socket&& sock, 
		bool accepted) = 0;

private:
	using key_t = uint16_t;
	using sequence = sequence<uint16_t>;
	using acceptors = std::map<key_t, tcp_acceptor::ptr>;
	using connectors = std::map<key_t, tcp_connector::ptr>;
	using protocols = slot_vector<protocol>;
	using channel_map = std::map<channel::key_t, channel::ptr>;

private:
	void on_accepted(key_t k, tcp::socket&& soc);

	/// called when accept failed
	void on_accept_failed(key_t k, const error_code& ec);

	/// called when connected
	void on_connected(key_t k, tcp::socket&& soc);

	/// called when connect failed
	void on_connect_failed(key_t k, const error_code& ec);

	// ���ο� ���ῡ�� �������� ����
	void on_new_socket(
		tcp::socket&& soc, 
		bool accepted);

private:
	mutable std::shared_mutex	mutex_;
	config				config_;
	sequence			seq_;
	acceptors			acceptors_;
	connectors			connectors_;
	protocols			protocols_;
};

} // kernel
} // wise