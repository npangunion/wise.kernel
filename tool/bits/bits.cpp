#include <pch.hpp>
#include <idl/idl_context.h>
#include <idl/idl_main.h>
#include <wise.kernel/core/logger.hpp>
#include <boost/program_options.hpp>
#include <iostream>

static constexpr char* VERSION = "0.0.1";

namespace po = boost::program_options;

int parse(const std::string& path);

int main(int argc, char* argv[])
{
	po::options_description description("bits idl compiler usage");

	description.add_options()
		("help,h", "Display this help message")
		("version,v", "Display the version number")
		("cplus", "Generate cplus code")
		("csharp", "Generate csharp code")
		("all,a", "Generate c++ and c# code")
		("force",  "Ignore file time and generate all IDLs included")
		("out,o", po::value<std::string>(), "Set code output folder")
		("file,f", po::value<std::string>(), "File to parse and generate code")
		("log,l", po::value<std::string>(), "Set log level [trace | debug | info]. Default level is info.")
		;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
	po::notify(vm);

	if (argc == 1 || vm.count("help"))
	{
		std::cout << description;
		return 0;
	}

	if (vm.count("version"))  
	{
		std::cout << VERSION << std::endl;
		return 0;
	}

	if (!vm.count("file"))
	{
		std::cout << "file must be provided" << std::endl;
		return -1;
	}

	WISE_INFO("go logger");

	idl_context::config cfg;

	if (vm.count("log"))
	{
		cfg.log_level = vm["log"].as<std::string>();
	}

	cfg.main_file = vm["file"].as<std::string>();

	if (vm.count("cplus"))
	{
		cfg.generate_cplus = true;
	}

	if (vm.count("csharp"))
	{
		cfg.generate_csharp = true;
	}

	if (vm.count("all"))
	{
		cfg.generate_cplus = true;
		cfg.generate_csharp = true;
	}

	if (vm.count("out"))
	{
		cfg.output_dir = vm["out"].as<std::string>();
	}

	return idl_context::inst().start(cfg);
}

