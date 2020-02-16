#pragma once 

#include <wise/base/stacktrace.hpp>
#include <exception>
#include <spdlog/fmt/fmt.h>

namespace wise
{

class exception : public std::exception
{
public:
	explicit exception(const char* const m, const char* file, int line) throw()
		: exception("exception",m, file,line)
	{
		stacktrace::dump(fmt::format("exception: {}/{}/{}", m, file, line).c_str());
	}

	explicit exception(const char* const m) throw()
	{
		stacktrace::dump(fmt::format("exception: {}", m).c_str());
	}

	virtual char const* what() const override
	{
		return desc_.c_str();
	}

protected: 
	explicit exception(const char* cls, char const* const m, const char* file, int line) throw()
		: std::exception(m)
	{
		desc_ = fmt::format("{}: {} on {}:{}", cls, std::exception::what(), file, line);
	}

private:
	std::string desc_;
};

} // wise

#define EXCEPTION(cls) \
class cls : public wise::exception  \
{ \
public: \
	explicit cls(const char* const m, const char* file, int line) throw() \
	: wise::exception(#cls, m, file, line) \
	{ \
	} \
};

#define WISE_THROW(m) 			throw wise::exception(m, __FILE__, __LINE__)
#define WISE_THROW_EX(cls, m) 	throw cls(m, __FILE__, __LINE__)
#define WISE_THROW_FMT(...)		throw wise::exception(fmt::format(__VA_ARGS__).c_str(), __FILE__, __LINE__)
#define WISE_THROW_IF(cond, m)	if ((cond)) { throw wise::exception(m, __FILE__, __LINE__); }