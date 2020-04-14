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
 * 기본 통신 기능을 제공하여 바이트를 송수신할 수 있다.
 * tcp 상에 메세지 흐름을 생성하는 프로토콜을 만든다.
 * @see msgpack_node
 */
class tcp_node : public node
{
public:
	friend class tcp_acceptor;
	friend class tcp_connector;
	friend class tcp_protocol;

	struct config : public node::config 
	{
		bool set_tcp_no_delay = true;			/// nagle disable
		bool set_tcp_linger = true;				/// SO_LINGER 켜기
		bool set_tcp_reuse_addr = true;			/// 실제 서비스에서는 끄는 것이 좋음
		std::size_t max_session_count = 5000;	/// 최대 세션 개수 제한
	};

public:
	tcp_node(const config& _config);

	~tcp_node();

	/// listen on addr with ip:port (ex. "0.0.0.0:7700") 
	result listen(const std::string& addr, channel::ptr ch);

	/// connect to addr with ip:port (ex. "127.0.0.1:7700") 
	result connect(const std::string& addr, channel::ptr ch);

	/// get protocol with an id. access w/ shared_mutex
	tcp_protocol::ptr get(protocol::id_t id);

	/// get tcp_node::config to check a current configuration
	const config& get_config() const
	{
		return config_;
	}

	/// internal. callled when an error occurrs in protocol
	void on_error(protocol::ptr p, const error_code& ec);

protected:
	result on_start() override;

	void on_finish() override;

	virtual protocol::ptr create_protocol(tcp::socket&& sock, bool accepted) = 0;
	virtual void notify_accepted(tcp_protocol::ptr p) = 0;
	virtual void notify_connected(tcp_protocol::ptr p) = 0;
	virtual void notify_connect_failed(const std::string& addr, const error_code& ec) = 0;
	virtual void notify_disconnect(tcp_protocol::ptr p, const error_code& ec) = 0;

private:
	using key_t = uint16_t;
	using sequence = sequence<uint16_t>;
	using acceptors = std::map<key_t, tcp_acceptor::ptr>;
	using connectors = std::map<key_t, tcp_connector::ptr>;
	using protocols = slot_vector<protocol>;
	using channel_map = std::map<channel::key_t, channel::ptr>;

private:
	void on_accepted(key_t k, tcp::socket&& soc, channel::ptr ch);

	/// called when accept failed
	void on_accept_failed(key_t k, const error_code& ec);

	/// called when connected
	void on_connected(key_t k, tcp::socket&& soc, channel::ptr ch);

	/// called when connect failed
	void on_connect_failed(key_t k, const error_code& ec);

	// 새로운 연결에서 프로토콜 생성
	void on_new_socket(
		tcp::socket&& soc,
		channel::ptr ch,
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