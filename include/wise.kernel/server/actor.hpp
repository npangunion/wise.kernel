#pragma once

#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/timer.hpp>
#include <wise.kernel/net/protocol.hpp>
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

	enum class mode
	{
		none,
		local,
		peer,
		client,
	};

	class ref
	{
	public:
		ref()
			: mode_(mode::none)
		{}

		ref(ptr _actor, bool is_client = false)
			: actor_(_actor)
		{
			if (is_client)
			{
				mode_ = mode::client;
			}
			else
			{
				mode_ = mode::peer;
			}
		}

		ref(protocol::ptr pp, id_t id)
			: mode_(mode::remote)
			, protocol_(pp)
		{
		}

		id_t get_id() const
		{
			WISE_ASSERT(mode_ != mode::none);

			if (mode_ == mode::local)
			{
				return actor_->get_id();
			}

			return id_;
		}

		bool send(packet::ptr p)
		{
			if (mode_ == mode::none)
			{
				return false;
			}

			if (mode_ == mode::local)
			{
				actor_->publish(p);
				return true;
			}
			else
			{
				auto rc = protocol_->send(p);
				return !!rc;
			}
		}

	private:
		ptr actor_;
		id_t id_ = 0;
		protocol::ptr protocol_;
		mode mode_;
	};

public:
	actor(server& _server, id_t parent, id_t id);

	actor(server& _server, id_t id);

	virtual ~actor();

	std::size_t publish(packet::ptr pp)
	{
		ch_.publish(pp);
	}

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
	
	virtual result run() { return result::success;  }

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