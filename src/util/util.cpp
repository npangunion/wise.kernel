#include <pch.hpp>

#include <wise.kernel/util/util.hpp>
#include <wise.kernel/core/macros.hpp>
#include <vector>

namespace wise {
namespace kernel {

std::vector<std::string> split(const std::string& str, const std::string& delims, unsigned int max_splits)
{
	std::vector<std::string>  ret;

	// Pre-allocate some space for performance
	ret.reserve(max_splits ? max_splits + 1 : 10);    // 10 is guessed capacity for most case

	unsigned int num_splits = 0;

	// Use STL methods 
	size_t start, pos;
	start = 0;
	do
	{
		pos = str.find_first_of(delims, start);
		if (pos == start)
		{
			// Do nothing
			start = pos + 1;
		}
		else if (pos == std::string::npos || (max_splits && num_splits == max_splits))
		{
			// Copy the rest of the string
			ret.push_back(str.substr(start));
			break;
		}
		else
		{
			// Copy up to delimiter
			ret.push_back(str.substr(start, pos - start));
			start = pos + 1;
		}
		// parse up to next real data
		start = str.find_first_not_of(delims, start);
		++num_splits;

	} while (pos != std::string::npos);

	return ret;
}

std::string remove_file_extension(const std::string& path)
{
	std::size_t last_index = path.find_last_of(".");

	if (last_index == std::string::npos)
	{
		return std::string();
	}

	return path.substr(0, last_index);
}

std::string get_filename(const std::string& path)
{
	std::size_t last_index = path.find_last_of("/");

	if (last_index == std::string::npos)
	{
		std::string nfile = path; // copy

		return nfile;
	}

	// has a bug when path does not have filename

	return path.substr(last_index + 1);
}

} // kernel
} // wise