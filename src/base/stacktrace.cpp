#include <pch.hpp>
#include <wise.kernel/base/stacktrace.hpp>
#include <wise.kernel/base/logger.hpp>

#if WISE_USE_STACKTRACE

#include <boost/stacktrace.hpp>
#include <sstream>

namespace wise {
	namespace kernel {

		constexpr int dump_trace_level = 10;

		std::string stacktrace::dump(const char* msg)
		{
			auto st = boost::stacktrace::stacktrace();

			std::ostringstream oss;

			int level = 0;

			oss << msg << std::endl;

			for (auto& f : st)
			{
				if (level >= 3) // skip dump
				{
					oss << "[" << f.name() << "] " << std::endl;;
					oss << "\t" << f.source_file() << ":" << f.source_line() << std::endl;
					oss << "\t" << "addr: " << f.address() << std::endl;
				}

				++level;

				if (level > dump_trace_level)
				{
					break;
				}
			}

			std::string str = oss.str();

			WISE_WARN("dump: {}", str);

			return str;
		}

	} // kernel
} // wise

#else 
namespace wise {
	namespace kernel {

		std::string stacktrace::dump(const char* msg)
		{
			WISE_WARN("{} stacktrace is not enabled.", msg);

			return std::string(msg);
		}

	} // kernel
} // wise


#endif