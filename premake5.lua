workspace "GameTechLabWeek2"
	configurations { "Debug", "Release" }

project "GameTechlabWeek2"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	characterset "Unicode"
	pchheader "pch.h"
	pchsource "GameTechlabWeek2/pch.cpp"
	
	targetdir "bin/%{cfg.buildcfg}"
	
	-- include를 상대 경로 대신 절대 경로로 쓸 수 있도록 수정
	-- ex. #include "../../GEngine.h" -> #include "Engine/GEngine.h"
	includedirs { "./GameTechlabWeek2/" } 
	
	externalincludedirs { "./GameTechlabWeek2/nlohmann" }

	files {
		"**.h",
		"**.cpp",
		"**.hpp",
		"**.c",
		"**.rc",
		"**.ico",
		"Assets/**",
		"Scenes/**"
	}

	-- 셰이더는 FRenderer에서 여러 진입점으로 런타임 컴파일한다.
	-- Visual Studio가 기본 진입점(main)으로 미리 컴파일하지 않도록 콘텐츠로만 취급한다.
	filter "files:**.hlsl"
		buildaction "None"

	filter {}

	-- 외부 라이브러리 소스는 자체 include 순서를 유지합니다.
	filter "files:**/ImGui/**.cpp"
		enablepch "Off"

	filter {}

	-- 멀티 프로세싱 컴파일
	-- 한번에 여러 cpp 파일 컴파일로 컴파일 속도 향상
	multiprocessorcompile "On"

	filter "toolset:msc*"
		-- 호출 규약이 팀원마다 다른(?) 기이한 버그 때문에 추가
		callingconvention "Cdecl"

		-- 한글 인코딩 문제를 UTF-8로 강제하여 해결
        buildoptions { "/utf-8" }
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		
	filter "configurations:Release"
		defines { "NDEBUG" }
		-- Release 빌드에서도 컴파일러/링커 최적화를 사용하지 않는다.
		-- 최적화된 바이너리가 일부 안티바이러스에서 오진되는 문제를 피하기 위함이다.
		optimize "Off"
		functionlevellinking "Off"
		intrinsics "Off"
		stringpooling "Off"
		linktimeoptimization "Off"

	filter { "configurations:Release", "toolset:msc*" }
		linkoptions { "/OPT:NOREF", "/OPT:NOICF" }
	
	-- 동적 링킹
	links {
		"d3d11",			-- DirectX11 
		"d3dcompiler",		-- DirectX11
		"dxgi",				-- DirectX11
		"user32"			-- Win32
	}

	-- Standalone test entry points are not part of the editor application.
	removefiles { "tests/**" }
