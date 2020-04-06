#include "stdafx.h"
#include "report.hpp"
#include <wise/base/logger.hpp>
#include <wise/base/macros.hpp>

namespace wise
{

report& report::inst()
{
	static report inst_;

	return inst_;
}

bool report::start(const std::string& file)
{
	std::ofstream ofs(file, std::ofstream::app);

	if (ofs.is_open())
	{
		ofs_.swap(ofs);

		write_head();

		WISE_INFO("report started");

		return true;
	}

	WISE_ERROR("failed to start report");

	return false;
}

void report::add(
	const std::string& agent,
	const std::string& category,
	const std::string& act,
	tick_t begin,
	tick_t end,
	tick_t elapsed,
	bool success)
{
	queue_.push({ agent, category, act, begin, end, elapsed, success });
}

void report::exec()
{
	record r; 

	while (queue_.pop(r))
	{
		write(r);
	}
}

void report::stop()
{
	if (ofs_.is_open())
	{
		exec();  // give chance to flush

		ofs_.flush();
		ofs_.close();

		WISE_INFO("report stopped");
	}
}

void report::write_head()
{
	WISE_EXPECT(ofs_.is_open());
	WISE_RETURN_IF(!ofs_.is_open());
	WISE_RETURN_IF(!enabled_);

	ofs_ 
		<< "agent," 
		<< "category," 
		<< "act," 
		<< "begin," 
		<< "end," 
		<< "elapsed," 
		<< "success" 
		<< std::endl;
}

void report::write(const record& r)
{
	WISE_EXPECT(ofs_.is_open());
	WISE_RETURN_IF(!ofs_.is_open());
	WISE_RETURN_IF(!enabled_);

	ofs_ 
		<< r.agent << "," 
		<< r.category << "," 
		<< r.act << "," 
		<< r.begin << "," 
		<< r.end << "," 
		<< r.elapsed << "," 
		<< r.success 
		<< std::endl;
}

} // wise
