#pragma once 

#include <wise/base/concurrent_queue.hpp>
#include <wise/base/tick.hpp>
#include <fstream>
#include <string>
#include <chrono>

namespace wise
{

class report
{
public: 
	static report& inst();

	bool start(const std::string& file);

	void add(
		const std::string& agent, 
		const std::string& category,
		const std::string& act, 
		tick_t begin_tick,	
		tick_t end_tick,	
		tick_t elapsed, 
		bool success);

	void exec();

	void stop();

	void disable()
	{
		enabled_ = false;
	}

private: 
	struct record
	{
		std::string agent; 
		std::string category; 
		std::string act; 
		tick_t begin;
		tick_t end;
		tick_t elapsed;
		bool success;
	};
	using record_queue = concurrent_queue<record>;

private:
	report() = default;
	report(const report&) = delete;

	void write_head();
	void write(const record& r);

private: 
	record_queue queue_;
	std::ofstream ofs_;
	bool enabled_ = true;
};

} // wise