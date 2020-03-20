#pragma once 

#include <idl/parse/idl_program.h>
#include <stack>
#include <unordered_map>

/** 
 * A context to keep the global parsing and generation state
 */
class idl_context
{
public: 
	struct config
	{
		bool continue_generation_on_error = true;
		bool generate_cplus = false;
		bool generate_csharp = false;
		std::string main_file = "main.bits";
		std::string log_level = "info";
		std::string output_dir = "";
	};

public:
	static idl_context& inst();

	~idl_context()
	{
	}

	int start(const config& _config);

	const config& get() const
	{
		return config_;
	}

	void push_program(idl_program::ptr p);

	void push_file(const std::string& f);

private: 
	struct file_state
	{
		std::string path;
		bool processed = false;

		file_state() = default;

		file_state(const std::string& _path, bool _processed)
			: path(_path)
			, processed(_processed)
		{
		}
	};

	using fmap = std::unordered_map<std::string, file_state>;

private: 
	idl_context() = default;

	int parse();

	int generate();

	void setup_config();

	void setup_config_log();

private: 
	bool started_ = false;
	config config_;

	std::stack<idl_program::ptr>	pstack_;
	std::stack<file_state>			fstack_;
	fmap							states_;
};