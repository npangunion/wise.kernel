#include <pch.hpp>

#include <wise.kernel/base/macros.hpp>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace wise {
	namespace kernel {

		/// cond에 따라 로그를 남긴다. 
		void check__(bool cond, const char* msg, const char* func, const char* file, int line)
		{
			WISE_ASSERT(!cond);
			WISE_UNUSED(cond);

			// datetime 

			std::ostringstream oss;


			std::time_t now_t = time(nullptr);
#ifdef _MSC_VER
			struct tm tm__;
			localtime_s(&tm__, &now_t);

			struct tm* tm_ = &tm__;
#else
			struct tm* tm_ = std::localtime(&now_t);
#endif

			oss
				<< "[" << tm_->tm_year + 1900 << "-" << tm_->tm_mon + 1 << "-" << tm_->tm_mday << " "
				<< tm_->tm_hour << ":" << tm_->tm_min << "]"
				<< "[" << std::this_thread::get_id() << "]"
				<< " failed: " << msg
				<< " [" << func << "]" << "[" << file << "@" << line << "]";

			std::ofstream f("check.log", std::ofstream::app);

			f << oss.str() << std::endl;

			f.close();
		}
	} 
} // wise
