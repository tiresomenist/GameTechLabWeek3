#include "pch.h"
#include <windows.h>

// ImGui 관련 헤더 삽입
#include "ImGui/imgui.h"

//렌더러 헤더파일
#include "Engine/GEngine.h"
#include "Engine/InputManager/WndProc.h"
#include "Engine/Log.h"

#include "../resource.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <exception>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 각종 메시지를 처리할 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ImGUI 메세지는 ImGUI가 처리
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    case WM_DESTROY:
        // Signal that the app should quit
        PostQuitMessage(0);
        break;
    case WM_SIZE: //lParam -> (Width|Height)=(LO|HI)
        {
        // 창 최소화 시 Width, Height가 0이므로 리사이즈하지 않음
        if (wParam == SIZE_MINIMIZED)
        {
            GDevice::GetInstance()->OnResize(0, 0);
            return 0;
        }

        const uint32 Width = static_cast<uint32>(LOWORD(lParam));
        const uint32 Height = static_cast<uint32>(HIWORD(lParam));

        try { GDevice::GetInstance()->OnResize(Width, Height); }
        catch (...) { PostQuitMessage(EXIT_FAILURE); }

        return 0;
        }
        break;
    default:
        return HandleInput(hWnd, message, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif
    // 윈도우 클래스 이름
    WCHAR WindowClass[] = L"JungleWindowClass";

    // 윈도우 타이틀바에 표시될 이름
    WCHAR Title[] = L"PEPE Engine";

    // 각종 메시지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
    WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    // 윈도우 클래스 등록
    wndclass.hInstance = hInstance;
    if (!RegisterClassW(&wndclass)) return EXIT_FAILURE;

    // 1024 x 1024 크기에 윈도우 생성
    HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
        nullptr, nullptr, hInstance, nullptr);

    // 엔진을 초기화합니다.
    GEngine* Engine = GEngine::GetInstance();
    int ExitCode = EXIT_SUCCESS;
    if (!hWnd) return EXIT_FAILURE;
    try
    {
        Engine->Initialize(hWnd);

        // Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
        bool bIsExit = false;
        while (bIsExit == false)
        {
            MSG msg;

            // 처리할 메시지가 더 이상 없을때 까지 수행
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                // 키 입력 메시지를 번역
                TranslateMessage(&msg);

                // 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 위에서 등록한 WndProc 으로 전달됨
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                {
                    ExitCode = static_cast<int>(msg.wParam);
                    bIsExit = true;
                    break;
                }
            }
            if (bIsExit) break;
            Engine->Tick();

        }

    }
    catch (const std::exception& Error)
    {
        OutputDebugStringA(Error.what());
        ExitCode = EXIT_FAILURE;
    }

    // 엔진을 정리합니다.
    Engine->Destroy();
    Engine = nullptr;
    if (IsWindow(hWnd)) DestroyWindow(hWnd);
    UnregisterClassW(WindowClass, hInstance);

    return ExitCode;
}
