local BOOST_HOME = os.getenv("BOOST_HOME")
local CATCH_HOME = os.getenv("CATCH_HOME")
local CATCH_INCLUDE_DIR = CATCH_HOME .. "/single_include/catch2"

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
	
function includeBOOST()
	includedirs ("$(BOOST_HOME)/include")
end	

function includeSPDLOG()
	includedirs ("$(SPDLOG_HOME)/include")
end	

project "wise.kernel"
	kind "StaticLib"
	includedirs "include"

	pchheader "pch.hpp"
	pchsource "src/pch.cpp"
	
	files { "include/**", "src/**" }

	buildoptions { "/std:c++17" }
	
	includeBOOST()
	includeSPDLOG()


function use_wise_kernel()
	includedirs "include"
	
	links "wise.kernel"
end    

function includeCatch()
	includedirs( CATCH_INCLUDE_DIR )
	defines "CATCH_CPP11_OR_GREATER"
end


-- The windowed app
project "wise.kernel.test"
	kind "ConsoleApp"
	files "test/**"

	buildoptions { "/std:c++17" }

	includedirs "include"
	includeCatch()

	use_wise_kernel()
	
    
