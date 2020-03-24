#pragma once 

#include <exception>
#include <sstream>
#include "logger.hpp" 

namespace wise {
namespace kernel {

class exception : public std::exception
{
public:
	explicit exception(const char* const m, const char* file, int line) throw()
		: exception("exception", m, file, line)
	{
	}

	explicit exception(const char* const m) throw()
		: std::exception(m)
	{
	}

	virtual char const* what() const override
	{
		return desc_.c_str();
	}

protected:
	explicit exception(const char* cls, char const* const m, const char* file, int line) throw()
		: std::exception(m)
	{
		desc_ = fmt::format("{}: {} on {} {}", cls, std::exception::what(), file, line);

		WISE_ERROR("{}", desc_);
	}

private:
	std::string desc_;
};

} // kernel
} // wise

#define EXCEPTION(cls) \
class cls : public wise::kernel::exception  \
{ \
public: \
	explicit cls(const char* const m, const char* file, int line) throw() \
	: wise::kernel::exception(#cls, m, file, line) \
	{ \
	} \
};

#define WISE_THROW(m) 			throw wise::kernel::exception(m, __FILE__, __LINE__)
#define WISE_THROW_EX(cls, m) 	throw cls(m, __FILE__, __LINE__)
#define WISE_THROW_FMT(...)		throw wise::kernel::exception(fmt::format(__VA_ARGS__).c_str(), __FILE__, __LINE__)
#define WISE_THROW_IF(cond, m)	if ((cond)) { throw wise::kernel::exception(m, __FILE__, __LINE__); }
#define WISE_THROW_IF_FMT(cond, ...)	if ((cond)) { throw wise::kernel::exception(fmt::format(__VA_ARGS__).c_str(), __FILE__, __LINE__); }