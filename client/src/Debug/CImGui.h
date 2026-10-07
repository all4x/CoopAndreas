#pragma once

class CImGui
{
public: 
	static void Init();
	static void SetActive(bool bActive);
	static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static inline bool ms_bActive = false;
	static inline bool ms_bDebugMenu = false; // D1212 cheat
	static inline bool ms_bCoopMenu = false;  // F7

	static void UpdateActive() { SetActive(ms_bDebugMenu || ms_bCoopMenu); }
};
