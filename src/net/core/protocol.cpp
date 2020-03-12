#include "stdafx.h"
#include <wise/net/protocol/protocol.hpp>
#include <wise/net/session.hpp>
#include <wise/base/logger.hpp>

namespace wise
{

protocol::protocol()
	: desc_("0:invalid")
{
}

protocol::~protocol()
{
}

bool protocol::bind(wptr ss)
{
	WISE_EXPECT(ss);
	WISE_RETURN_IF(!ss, false);

	WISE_ASSERT(!session_);
	WISE_RETURN_IF(session_, false);

	session_ = ss;

	desc_ = fmt::format(
		"protocol:{0}:{1}", 
		ss->get_id().get_index(), 
		ss->get_remote_addr()
	);

	on_bind();

	return !!session_;
}

protocol::result protocol::send(const uint8_t* const data, std::size_t len)
{
	WISE_RETURN_IF(!get_session(), result( false, reason::fail_invalid_session ));

	return get_session()->send(data, len);
}

void protocol::close()
{
	WISE_INFO("close from protocol. {}", get_session()->get_desc());

	get_session()->close();
}

} // wise