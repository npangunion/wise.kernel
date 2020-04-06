local BOOST_HOME = os.getenv("BOOST_HOME")
local SPDLOG_HOME = os.getenv("SPDLOG_HOME")
local CATCH_HOME = os.getenv("CATCH_HOME")
local CATCH_INCLUDE_DIR = CATCH_HOME .. "/single_include/catch2"
local BITSERY_HOME = os.getenv("BITSERY_HOME")
local BOTAN_HOME = os.getenv("BOTAN_HOME")

workspace "wise.kernel"
	location "generated"
	language "C++"
	architecture "x86_64"
	
	configurations { "Debug", "Release" }
	
	filter { "configurations:Debug" }
		symbols "On"
	
	filter { "configurations:Release" }
		optimize "On"
	
	filter { }

	
	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
    objdir ("build/obj/%{prj.name}/%{cfg.longname}")
	
function useBOOST()
	includedirs (BOOST_HOME)
	libdirs (BOOST_HOME .. "/stage/lib")
end	

function includeSPDLOG()
	includedirs (SPDLOG_HOME .. "/include")
end	

function includeBITSERY()
	includedirs (BITSERY_HOME .. "/include")
end	

function useBOTAN()
	filter "architecture:x86_64"
		includedirs (BOTAN_HOME .. "/x64/include")
		libdirs (BOTAN_HOME .. "/x64/lib")
	
	filter "configurations:Debug"
		links "botan_d"

	filter "configurations:Release"
		links "botan"

	filter {}
end

project "wise.kernel"
	kind "StaticLib"
	includedirs "include"

	pchheader "pch.hpp"
	pchsource "src/pch.cpp"
	
	files { "include/**", "src/**" }

	excludes { "src/core/botan/arch/**" }
	excludes { "include/wise.kernel/server/service/db/**" }
	excludes { "include/wise.kernel/server/service/bot/**" }
	excludes { "src/service/server/db/**" }
	excludes { "src/service/server/bot/**" }

	defines "_ENABLE_EXTENDED_ALIGNED_STORAGE"
	warnings "extra"
	buildoptions { "/std:c++17" }
	flags { "MultiProcessorCompile" }

	staticruntime "On"
	
	useBOOST()
	includeSPDLOG()


	configuration "Debug"
		targetdir "lib"
		targetname "wise.kerneld"

	configuration "Release"
		targetdir "lib"
		targetname "wise.kernel"

	useBOTAN()


function use_wise_kernel()
	includedirs "include"

	filter "configurations:Debug"
		links "wise.kernel"
	
	filter "configurations:Release"
		links "wise.kernel"

	filter {}
end    

function includeCatch()
	includedirs( CATCH_INCLUDE_DIR )
	defines "CATCH_CPP11_OR_GREATER"
end


-- The windowed app
project "wise.kernel.test"
	kind "ConsoleApp"

	files "test/**"

	pchheader "pch.hpp"
	pchsource "test/pch.cpp"

	warnings "extra"
	buildoptions { "/std:c++17" }
	flags { "MultiProcessorCompile" }

	staticruntime "On"

	includedirs "test"
	includeCatch()

	use_wise_kernel()

	useBOOST()
	useBOTAN()
	includeSPDLOG()
	
