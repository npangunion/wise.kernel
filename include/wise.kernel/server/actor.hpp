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
	using ptr = std::shared_ptr<actor>;

public:
	actor(server& _server, id_t parent, id_t id);

	actor(server& _server, id_t id);

	virtual ~actor();

	id_t get_id() const
	{
		return id_;
	}

	id_t get_parent() const
	{
		return parent_;
	}

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

	channel& get_channel() const
	{
		return ch_;
	}

	timer& get_timer() const 
	{
		return timer_;
	}

private: 
	/// task::on_start(). call init()
	bool on_start() override;

	/// task::on_execute(). ch_.publish(), timer ½ÇÇà
	result on_execute() override;

	/// task::on_finish(). call fini()
	void on_finish() override;

private: 
	server&			server_;
	const id_t		parent_ = 0;
	const id_t		id_ = 0;
	mutable channel	ch_;
	mutable timer	timer_;
};

} // kernel
} // wise