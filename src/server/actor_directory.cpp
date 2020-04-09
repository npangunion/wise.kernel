#include <pch.hpp>
#include <wise.kernel/server/actor_directory.hpp>

namespace wise
{
namespace kernel
{

actor_directory::actor_directory()
{

}

actor_directory::~actor_directory()
{
	cleanup();
}

void actor_directory::cleanup()
{
	// scheduler finishes an actor as a task
	// here, we just clear explicitly
	actors_.clear();
}

} // kernel
} // wise