#include "stdafx.h"
#include <wise/service/db/tx.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

namespace wise
{

tx::tx(const topic& pic)
	: hx(pic)
{
	auto uid = boost::uuids::random_generator()();
	ctx_.id = boost::uuids::to_string(uid);
}

bool tx::context::pack(zen_packer& packer) const
{
	packer.pack(dir);
	packer.pack(id);
	packer.pack(db);
	packer.pack(key);
	packer.pack(oid);
	packer.pack(channel);
	packer.pack_enum(rc);
	packer.pack(error);
	packer.pack(query_time);

	return packer.is_valid();
}

bool tx::context::unpack(zen_packer& packer)
{
	packer.unpack(dir);
	packer.unpack(id);
	packer.unpack(db);
	packer.unpack(key);
	packer.unpack(oid);
	packer.unpack(channel);
	packer.unpack_enum(rc);
	packer.unpack(error);
	packer.unpack(query_time);

	return packer.is_valid();
}

std::string tx::context::get_desc() const
{
	return fmt::format(
		"dir:{}, id:{}, db:{}, key:{}, oid:{}, channel:{}, error: {}",
		dir, id, db, key, oid, channel, error
	);
}

} // wise
