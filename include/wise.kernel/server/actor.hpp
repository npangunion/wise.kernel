#pragma once

#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/timer.hpp>
#include <stdint.h>

namespace wise
{
namespace kernel
{

class server;

class actor : public task
{
public: 
	using id_t = uint64_t;

public:
	actor(server& _server, id_t parent, id_t id);

	actor(server& _server, id_t id);

	virtual ~actor();

protected:
	virtual bool init() = 0;

	virtual void fini() = 0;

	server& get_server()
	{
		return server_;
	}

	const server& get_server() const
	{
		return server_;
	}

private: 
	/// task::on_start(). call init()
	bool on_start() override;

	/// task::on_execute(). ch_.publish(), timer ½ÇÇà
	result on_execute() override;

	/// task::on_finish(). call fini()
	void on_finish() override;

private: 
	server& server_;
	channel ch_;
	timer timer_;
};

} // kernel
} // wise