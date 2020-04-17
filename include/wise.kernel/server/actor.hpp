#pragma once

#include <wise.kernel/core/channel/channel.hpp>
#include <wise.kernel/core/task/task.hpp>
#include <wise.kernel/core/timer.hpp>
#include <wise.kernel/net/protocol.hpp>
#include <wise.kernel/util/json.hpp>
#include <stdint.h>

namespace wise
{
namespace kernel
{

class server;

/// actor with a channel running as a task
class actor : public task
{
public: 
	using id_t = uint64_t;
	using ptr = std::shared_ptr<actor>;

	enum class mode : uint8_t
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

		ref(ptr _actor)
			: actor_(_actor)
			, mode_(mode::local)
		{
			WISE_THROW_IF(!actor_, "actor is not valid");
			id_ = actor_->get_id();
		}

		ref(protocol::ptr pp, id_t id, bool is_client = false)
			: protocol_(pp)
			, id_(id)
		{
			WISE_THROW_IF(id_ == 0, "actor id is not valid");

			if (is_client)
			{
				mode_ = mode::client;
			}
			else
			{
				mode_ = mode::peer;
			}
		}

		id_t get_id() const
		{
			WISE_ASSERT(mode_ != mode::none);

			if (mode_ == mode::local)
			{
				WISE_ASSERT(actor_->get_id() == id_);
			}

			return id_;
		}

		mode get_mode() const
		{
			return mode_;
		}

		bool is_local() const
		{
			return mode_ == mode::local;
		}

		bool is_remote() const
		{
			return mode_ == mode::peer;
		}

		bool is_client() const
		{
			return mode_ == mode::client;
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

		void set_name(const std::string& name)
		{
			name_ = name;
		}

		const std::string& get_name() const
		{
			return name_;
		}

		bool has_name() const
		{
			return name_.length() > 0;
		}

		bool operator==(const ref& rhs) const
		{
			return mode_ == rhs.mode_ &&
				actor_ == rhs.actor_ &&
				id_ == rhs.id_ &&
				protocol_ == rhs.protocol_;
		}

		operator bool() const
		{
			WISE_RETURN_IF(id_ == 0, false);

			if (mode_ == mode::local)
			{
				return !!actor_;
			}

			return !!protocol_;
		}

	private:
		ptr				actor_;
		id_t			id_ = 0;
		protocol::ptr	protocol_;
		mode			mode_;
		std::string		name_;
	};

public:
	/// constructor
	actor(server& _server, id_t id);

	/// destructor
	virtual ~actor();

	/// setup with json configuration
	virtual bool setup(const nlohmann::json& _json);

	/// publish a packet to my channel
	std::size_t publish(packet::ptr pp)
	{
		ch_->publish(pp);
	}

	/// get id 
	id_t get_id() const
	{
		return id_;
	}

protected:
	/// initialize 
	virtual bool init() = 0;
	
	/// called from execute in task_runner
	virtual result run() { return result::success;  }

	/// finish
	virtual void fini() = 0;

	server& get_server()
	{
		return server_;
	}

	const server& get_server() const
	{
		return server_;
	}

	channel::ptr get_channel() const
	{
		return ch_;
	}

	timer& get_timer() const 
	{
		return timer_;
	}

	template <typename EVT> 
	std::shared_ptr<EVT> cast(message::ptr m)
	{
		return std::static_pointer_cast<EVT>(m);
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
	const id_t		id_ = 0;
	channel::ptr	ch_;
	mutable timer	timer_;
};

} // kernel
} // wise

#define WISE_SUBSCRIBE_SELF(evt, func) \
	get_channel()->subscribe(evt::get_topic(), WISE_CHANNEL_CB(func))

