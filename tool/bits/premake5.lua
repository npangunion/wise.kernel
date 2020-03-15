local BOOST_HOME = os.getenv("BOOST_HOME")
local SPDLOG_HOME = os.getenv("SPDLOG_HOME")
local CATCH_HOME = os.getenv("CATCH_HOME")
local CATCH_INCLUDE_DIR = CATCH_HOME .. "/single_include/catch2"
local BITSERY_HOME = os.getenv("BITSERY_HOME")

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

function use_wise_kernel()
	includedirs "include"
	
	links "wise.kernel"
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

	warnings "extra"
	buildoptions { "/std:c++17" }

	includedirs  "." 

	includeBOOST()
	includeSPDLOG()
	
    
