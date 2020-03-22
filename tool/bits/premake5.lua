local BOOST_HOME = os.getenv("BOOST_HOME")
local SPDLOG_HOME = os.getenv("SPDLOG_HOME")
local CATCH_HOME = os.getenv("CATCH_HOME")
local CATCH_INCLUDE_DIR = CATCH_HOME .. "/single_include/catch2"
local BITSERY_HOME = os.getenv("BITSERY_HOME")
local WISE_KERNEL_HOME = "../../"

workspace "wise.bits"
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
	
function includeBOOST()
	includedirs (BOOST_HOME)
end	

function includeSPDLOG()
	includedirs (SPDLOG_HOME .. "/include")
end	

function includeBITSERY()
	includedirs (BITSERY_HOME .. "/include")
end	

function useWISE_KERNEL()
	includedirs (WISE_KERNEL_HOME .. "/include")
	libdirs (WISE_KERNEL_HOME .. "/lib")
	
	filter { "configurations:Debug" }
		links "wise.kerneld"

	filter { "configurations:Release" }
		links "wise.kernel"

	filter {}
end    

function includeCatch()
	includedirs( CATCH_INCLUDE_DIR )
	defines "CATCH_CPP11_OR_GREATER"
end


-- The windowed app
project "wise.bits"
	kind "ConsoleApp"

	files "idl/**"
	files("bits.cpp")
	files("pch.cpp")
	files("pch.hpp")

	pchheader "pch.hpp"
	pchsource "pch.cpp"

	includeBOOST()
	includeSPDLOG()

	includedirs  "." 
	useWISE_KERNEL()

	staticruntime "On"

	warnings "extra"
	buildoptions { "/std:c++17" }
	
	filter { 'files:idl/idl_parser.cpp' }
		flags { 'NoPCH' }

	filter { 'files:idl/idl_lexer.cpp' }
		flags { 'NoPCH' }
    
