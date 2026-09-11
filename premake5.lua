workspace "GameTechLabWeek2"
	configurations { "Debug", "Release" }

project "GameTechlabWeek2"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	characterset "Unicode"
	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	targetdir "bin/%{cfg.buildcfg}"

	-- include는 Source 기준 경로로 쓴다. 같은 폴더의 헤더만 파일 이름으로 쓴다.
	-- ex. #include "Engine/Renderer/FRenderer.h", #include "Core/Math/FVector.h"
	-- 의존 방향: Core <- Engine <- Editor
	includedirs { "./Source/" }

	-- 외부 라이브러리: #include "ImGui/imgui.h", #include "nlohmann/json.hpp"
	externalincludedirs { "./ThirdParty/" }

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
	
    -- 위의 Release 전용 필터를 해제한다.
    filter {}

    -- 설치한 directxtex:x86-windows와 맞춘다.
    architecture "x86"
    staticruntime "Off"

   local vcpkgRoot = os.getenv("VCPKG_ROOT")

if not vcpkgRoot or vcpkgRoot == "" then
    error("VCPKG_ROOT 환경변수에 vcpkg 설치 경로를 지정하세요.")
end

local vcpkgDir = path.join(
    vcpkgRoot,
    "installed/x86-windows"
)

    -- #include <DirectXTex.h>를 찾을 위치
    externalincludedirs {
        vcpkgDir .. "/include"
    }

    -- 모든 구성에서 연결할 라이브러리
    links {
        "d3d11",
        "d3dcompiler",
        "dxgi",
        "user32",
        "DirectXTex",
        "windowscodecs",
        "ole32"
    }

    filter "configurations:Debug"
        runtime "Debug"
        libdirs {
            vcpkgDir .. "/debug/lib"
        }

    filter "configurations:Release"
        runtime "Release"
        libdirs {
            vcpkgDir .. "/lib"
        }

    filter {}

    -- 테스트 소스는 엔진 실행 파일에 포함하지 않는다.
    removefiles { "tests/**" }
