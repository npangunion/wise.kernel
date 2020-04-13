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
	actor(server& _server, id_t id);

	virtual ~actor();

	virtual bool setup(const nlohmann::json& _json);

	std::size_t publish(packet::ptr pp)
	{
		ch_.publish(pp);
	}

	id_t get_id() const
	{
		return id_;
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
	const id_t		id_ = 0;
	mutable channel	ch_;
	mutable timer	timer_;
};

} // kernel
} // wise