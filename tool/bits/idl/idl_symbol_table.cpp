#include <pch.hpp>
#include "idl_symbol_table.hpp"
#include <wise.kernel/core/logger.hpp>


idl_symbol_table::result 
idl_symbol_table::add(const idl_symbol& sym)
{
	auto iter = symbols_.find(sym.full_name_);

	if (iter != symbols_.end())
	{
		WISE_ERROR("duplicate symbol {}", sym.full_name_);
		return result(false, sym);
	}

	symbols_[sym.full_name_] = sym;

	return result(true, sym);
}

idl_symbol_table::result 
idl_symbol_table::lookup(const std::string& full_symbol)
{
	auto iter = symbols_.find(full_symbol);

	if (iter == symbols_.end())
	{
		return result(false, idl_symbol{});
	}

	return result(true, iter->second); // copied. ok?
}

idl_symbol_table::result 
idl_symbol_table::lookup(const std::string& ns, const std::string& simple)
{
	auto full_symbol = ns + "::" + simple; // c++ 이름을 표준으로 검색한다.

	auto iter = symbols_.find(full_symbol);

	if (iter == symbols_.end())
	{
		return result(false, idl_symbol{});
	}

	return result(true, iter->second); // copied. ok?
}
