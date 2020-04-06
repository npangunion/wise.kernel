#pragma once

#include <wise/service/bot/act.hpp>
#include <wise/net/session.hpp>

namespace wise
{

/// 연결. agent oid 바인딩
class act_connect: public act
{
public: 
	act_connect(flow& owner, config& cfg);

private: 
	void on_begin() override; 

	void on_session_ready(message::ptr mp);

	void on_connect_failed(message::ptr mp);

	void load();

private:
	std::string name_;
	std::string addr_;
	session::sid_t sid_ = session_ref::invalid_id;
};

} // wise

