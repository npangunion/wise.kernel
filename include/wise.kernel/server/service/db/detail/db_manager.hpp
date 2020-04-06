#pragma once

namespace lax
{
namespace db
{

/// db connection manager
class db_manager
{
public:
	static db_manager& get();

	~db_manager();

	void add();

	

private:

};

} // db
} // lax