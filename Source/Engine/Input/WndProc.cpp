#include "pch.h"
#include "WndProc.h"
#include "Engine/Input/GInputManager.h"
#include "Core/Core.h"
#include "Engine/Log.h"
#include "ImGui/imgui.h"

#include <windowsx.h>

LRESULT HandleInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GInputManager& Input = *GInputManager::GetInstance();
	RECT rc;

	bool DisableMouse = false;
	bool DisableKeyboard = false;
	
	if (ImGui::GetCurrentContext())
	{
		ImGuiIO& io = ImGui::GetIO();

		DisableMouse = io.WantCaptureMouse;
		DisableKeyboard = io.WantCaptureKeyboard;
	}


	if (message == WM_KEYDOWN || message == WM_KEYUP || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP)
	{
		if (DisableKeyboard && message == WM_KEYDOWN)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		GInputManager::EInputStatus Key = GInputManager::KEY_COUNT;

		switch (wParam)
		{
		case 'W': { Key = GInputManager::EI_W; break; }
		case 'A': { Key = GInputManager::EI_A; break; }
		case 'S': { Key = GInputManager::EI_S; break; }
		case 'D': { Key = GInputManager::EI_D; break; }
		case 'Q': { Key = GInputManager::EI_Q; break; }
		case 'E': { Key = GInputManager::EI_E; break; }
		case VK_SPACE: { Key = GInputManager::EI_SPACE; break; }
		}

		if (Key == GInputManager::KEY_COUNT)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
		{
			Input.SetKey(Key, true);
		}
		else
		{
			Input.SetKey(Key, false);
		}

		return 0;
	}

	if(
		message == WM_LBUTTONDOWN ||
		message == WM_LBUTTONUP ||
		message == WM_RBUTTONDOWN ||
		message == WM_RBUTTONUP ||
		message == WM_MOUSEMOVE
		)
	{
		const bool bContinuingDrag =
			(message == WM_LBUTTONUP && Input.GetKey(GInputManager::EI_LMOUSE)) ||
			(message == WM_RBUTTONUP && Input.GetKey(GInputManager::EI_RMOUSE)) ||
			(message == WM_MOUSEMOVE && (Input.GetKey(GInputManager::EI_LMOUSE) || Input.GetKey(GInputManager::EI_RMOUSE)));
		if (DisableMouse && !bContinuingDrag)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		
		switch (message)
		{
		case WM_LBUTTONDOWN:

			GetClientRect(hWnd, &rc);
			Input.SetLeftCursorX(2.0f * GET_X_LPARAM(lParam) / (rc.right - rc.left) - 1.0f);
			Input.SetLeftCursorY(1.0f - 2.0f * GET_Y_LPARAM(lParam) / (rc.bottom - rc.top));
			Input.SetLeftCursorPixelX(GET_X_LPARAM(lParam));
			Input.SetLeftCursorPixelY(GET_Y_LPARAM(lParam));
			Input.SetKey(GInputManager::EI_LMOUSE, true);
			Input.BeginLeftDrag(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (GetCapture() != hWnd)
				SetCapture(hWnd);
			break;
		case WM_LBUTTONUP:
			Input.UpdateLeftDrag(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			Input.SetKey(GInputManager::EI_LMOUSE, false);
			if (GetCapture() == hWnd && !Input.GetKey(GInputManager::EI_RMOUSE)) ReleaseCapture();
			break;
		case WM_RBUTTONDOWN:
			GetClientRect(hWnd, &rc);
			Input.SetRightCursorX(2.0f * GET_X_LPARAM(lParam) / (rc.right - rc.left) - 1.0f);
			Input.SetRightCursorY(1.0f - 2.0f * GET_Y_LPARAM(lParam) / (rc.bottom - rc.top));
			Input.SetRightCursorPixelX(GET_X_LPARAM(lParam));
			Input.SetRightCursorPixelY(GET_Y_LPARAM(lParam));
			Input.BeginRightDrag(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (GetCapture() != hWnd)
				SetCapture(hWnd);
			break;
		case WM_RBUTTONUP:
			GetClientRect(hWnd, &rc);
			Input.SetRightCursorX(2.0f * GET_X_LPARAM(lParam) / (rc.right - rc.left) - 1.0f);
			Input.SetRightCursorY(1.0f - 2.0f * GET_Y_LPARAM(lParam) / (rc.bottom - rc.top));
			Input.SetRightCursorPixelX(GET_X_LPARAM(lParam));
			Input.SetRightCursorPixelY(GET_Y_LPARAM(lParam));
			Input.EndRightDrag();
			if (GetCapture() == hWnd && !Input.GetKey(GInputManager::EI_LMOUSE)) ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			// 우클릭 드래그중
			if (Input.GetKey(GInputManager::EI_RMOUSE)) {
				GetClientRect(hWnd, &rc);
				Input.SetRightCursorX(2.0f * GET_X_LPARAM(lParam) / (rc.right - rc.left) - 1.0f);
				Input.SetRightCursorY(1.0f - 2.0f * GET_Y_LPARAM(lParam) / (rc.bottom - rc.top));
				Input.UpdateRightDrag(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
			// 좌클릭 드래그중
			if (Input.GetKey(GInputManager::EI_LMOUSE)) {
				Input.UpdateLeftDrag(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
			break;
		}

		return 0;
	}

	switch (message)
	{
	case WM_CAPTURECHANGED:
		Input.EndRightDrag();
		if (Input.GetKey(GInputManager::EI_LMOUSE)) Input.KillFocus();
		break;
	case WM_CANCELMODE:
	case WM_KILLFOCUS:
		Input.KillFocus();
		if (GetCapture() == hWnd) ReleaseCapture();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
