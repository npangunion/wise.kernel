#pragma once 

#include <wise/net/session.hpp>
#include <wise/net/reason.hpp>

#include <functional>

namespace wise
{

class network_impl;

/// a singleton to provide networking network
/** 
 * usage: 
 *  - listen to start listen on an address with a protocol 
 *  - connect to connect to an address with a protocol 
 *  - session::sub for messages 
 *    - sys_session_ready / sys_session_closed for sessions
 *    - sys_connect_failed to detect failure to connect to servers   
 *  - acquire to get a session_ref 
 *    - this is the only way to send to a session
 *    - id based access becomes hell in communication
 *      - lots of timing / duration issues with id reuse
 *    - ref::send, ref::close
 *  - session::sub for a session close through session_ref
 */
class network final
{
public: 
	using result = result<bool, reason>;

	struct config
	{
		bool use_hardware_concurreny = true;
		int concurreny_level = 1;				/// core���� ������� ���� ���
		bool enable_detail_log = false;			/// �ڼ��� �α� ������ ����
		bool set_tcp_no_delay = true;			/// nagle disable
		bool set_tcp_linger = true;				/// SO_LINGER �ѱ�
		bool set_tcp_reuse_addr = true;			/// ���� ���񽺿����� ���� ���� ����
		bool throw_on_subscription_to_invalid_session = true;
	};

	static config cfg;

	using protocol_creator = std::function<protocol::ptr()>;
	using cond_t = channel::cond_t;
	using cb_t = channel::cb_t;

public:
	/// subscribe to a topic with a condition 
	/**
	* direct mode subscription only. packet is posted immediately
	*/
	static sub::key_t subscribe(
		const topic& topic,
		cond_t cond,
		cb_t cb
	);

	static sub::key_t subscribe(
		const topic::key_t key,
		cond_t cond,
		cb_t cb
	)
	{
		return subscribe(topic(key), cond, cb);
	}

	/// subscribe to a topic without condition 
	/**
	* direct mode subscription only. packet is posted immediately
	*/
	static sub::key_t subscribe(
		const topic& topic,
		cb_t cb
	);

	static sub::key_t subscribe(
		const topic::key_t key,
		cb_t cb
	)
	{
		return subscribe(topic(key), cb);
	}

	/// unsubscribe 
	static bool unsubscribe(sub::key_t key);

	/// ������� �޼����� channel�� ����
	static void post(packet::ptr m);

	/// ���߱� ���� ä�ε� ���� ����.  
	static void stop_post_to_finish();

	/// for debugging purposes (unit test) 
	static void clear_subscription();

public:
	/// ���Ǹ� ���� �̱��� 
	static network& inst();

	/// ��� ���� �������� ��� �ʿ�
	static void add_protocol(const std::string& name, protocol_creator& creator);

public:
	~network();

	/// start
	bool start();

	/// finish
	void finish();

	/// wait for ms
	void wait(unsigned int ms);

	/// addr���� listen. addr�� ip:port ����. 
	result listen(const std::string& addr, const std::string& protocol, uintptr_t pkey = 0);

	/// connect to a addr. addr�� ip:port ����
	result connect(const std::string& addr, const std::string& protocol, uintptr_t pkey = 0);

	/// get a ref to send and subscribe for close / error
	session_ref acquire(const session::id& id);

	/// ���� �Լ�
	session_ref acquire(uint32_t id)
	{
		return acquire(session::id(id));
	}

	/// check whether serivce is started
	bool is_running() const;

	/// get acceptor count. 
	uint16_t get_acceptor_count() const;

	/// get connector count. 
	uint16_t get_connector_count() const;

	/// get # of sessions (for debug and test)
	std::size_t get_session_count() const;

	/// session���� ���� �߻� �� ȣ��
	void destroy(const session::id& id, const asio::error_code& ec);

	/// internal use only
	network_impl& impl() { return *impl_.get(); }

private: 
	network();

private: 
	std::recursive_mutex  mutex_;		// to protect start, finish
	std::unique_ptr<network_impl>	impl_;
};

} // wise

// session acquire. �����ؼ� ���.
#define WiSE_GET_SESSION(sid) ::wise::network::inst().acquire((sid))
