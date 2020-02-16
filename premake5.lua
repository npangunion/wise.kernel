local BOOST_HOME = os.getenv("BOOST_HOME")
local CATCH_HOME = os.getenv("CATCH_HOME")
local CATCH_INCLUDE_DIR = CATCH_HOME .. "/single_include/catch2"

workspace "wise.kernel"
	-- We set the location of the files Premake will generate
	location "generated"
	
	-- We indicate that all the projects are C++ only
	language "C++"
	
	-- We will compile for x86_64. You can change this to x86 for 32 bit builds.
	architecture "x86_64"
	
	-- Configurations are often used to store some compiler / linker settings together.
    -- The Debug configuration will be used by us while debugging.
    -- The optimized Release configuration will be used when shipping the app.
	configurations { "Debug", "Release" }
	
	-- We use filters to set options, a new feature of Premake5.
	
	-- We now only set settings for the Debug configuration
	filter { "configurations:Debug" }
		-- We want debug symbols in our debug config
		symbols "On"
	
	-- We now only set settings for Release
	filter { "configurations:Release" }
		-- Release should be optimized
		optimize "On"
	
	-- Reset the filter for other settings
	filter { }
	
	-- Here we use some "tokens" (the things between %{ ... }). They will be replaced by Premake
	-- automatically when configuring the projects.
	-- * %{prj.name} will be replaced by "ExampleLib" / "App" / "UnitTests"
	--  * %{cfg.longname} will be replaced by "Debug" or "Release" depending on the configuration
	-- The path is relative to *this* folder
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
	
	files { "include/**", "src/**" }
	
	includeBOOST()
	includeSPDLOG()


function use_wise_kernel()
	-- The library's public headers
	includedirs "include"
	
	-- We link against a library that's in the same workspace, so we can just
	-- use the project name - premake is really smart and will handle everything for us.
	links "wise.kernel"
	
	-- Users of ExampleLib need to link GLFW
	-- link()
end    

function includeCatch()
	includedirs( CATCH_INCLUDE_DIR )
	defines "CATCH_CPP11_OR_GREATER"
end


-- The windowed app
project "wise.kernel.test"
	kind "ConsoleApp"
	files "test/**"

	includedirs "include"
	includeCatch()

	use_wise_kernel()
	
    
