#include "CImGui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"
#include "CPacketTimeline.h"
#include "MissionRunner.h"
#include "UI/CCoopMenu.h"

ImFont* pFont;

void InitFonts()
{
    char windowsDir[MAX_PATH];
    GetWindowsDirectoryA(windowsDir, MAX_PATH);
    std::string fontsDir = std::string(windowsDir) + "\\Fonts\\";

    ImGuiIO& io = ImGui::GetIO();
    pFont = nullptr;

    std::string segoePath = fontsDir + "segoeui.ttf";
    if (FileExists(segoePath.c_str()))
    {
        pFont = io.Fonts->AddFontFromFileTTF(segoePath.c_str(), 16.0f);
    }

    if (!pFont)
    {
        pFont = io.Fonts->AddFontDefault();
    }
}

void InitStyles()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    style.Colors[ImGuiCol_WindowBg] = ImColor(13, 19, 33, 255);
    style.Colors[ImGuiCol_ChildBg] = ImColor(24, 31, 47, 255);
    style.ChildRounding = 3.0f;
}

// While a menu is open the game must not grab the mouse, otherwise the camera
// keeps turning and the cursor is re-centered every frame (can't click).
//   0x6194A0: jmp psSetMousePos (calls SetCursorPos)        -> ret
//   0x541DD7: call CPad::UpdateMouse (0x53F3C0) in the game loop -> nop
// Addresses verified in the 1.0 US exe disassembly; 0x53F3C0 matches plugin-sdk.
static void SetGameMouseGrab(bool bGrab)
{
    static bool bSaved = false;
    static uint8_t origSetMousePos[1];
    static uint8_t origUpdateMouseCall[5];

    if (!bSaved)
    {
        patch::GetRaw(0x6194A0, origSetMousePos, 1);
        patch::GetRaw(0x541DD7, origUpdateMouseCall, 5);
        bSaved = true;
    }

    if (bGrab)
    {
        patch::SetRaw(0x6194A0, origSetMousePos, 1);
        patch::SetRaw(0x541DD7, origUpdateMouseCall, 5);
    }
    else
    {
        patch::SetUChar(0x6194A0, 0xC3);
        patch::Nop(0x541DD7, 5);
        CPad::NewMouseControllerState = CMouseControllerState();
        CPad::OldMouseControllerState = CMouseControllerState();
    }
}

void CImGui::SetActive(bool bActive)
{
    if (ms_bActive != bActive)
        SetGameMouseGrab(!bActive);

    ms_bActive = bActive;

    if (ms_bActive)
    {
        CPad::GetPad(0)->DisablePlayerControls |= 0x200;
    }
    else
    {
        CPad::GetPad(0)->DisablePlayerControls &= ~0x200;
    }

    static_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice())->ShowCursor(ms_bActive);
    ImGui::GetIO().MouseDrawCursor = ms_bActive;
}

void CImGui::Init()
{
    Events::initRwEvent += []
    {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(RsGlobal.ps->window);
        ImGui_ImplWin32_EnableDpiAwareness();

        ImGui_ImplDX9_Init(static_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice()));

        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = false;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename = nullptr;

        InitStyles();
        InitFonts();

        ImGui_ImplDX9_InvalidateDeviceObjects();
        ImGui_ImplDX9_CreateDeviceObjects();
    };

    Events::drawAfterFadeEvent += []
    {
        if (CImGui::ms_bActive && !FrontEndMenuManager.m_bMenuActive)
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
            {
                CPad::GetPad(0)->DisablePlayerControls |= 0x200;
            }
            else
            {
                CPad::GetPad(0)->DisablePlayerControls &= ~0x200;
            }

            ImGui::PushFont(pFont);

            if (CImGui::ms_bCoopMenu && !CCoopMenu::Draw())
            {
                CImGui::ms_bCoopMenu = false;
                CImGui::UpdateActive();
            }

            if (CImGui::ms_bDebugMenu)
            {
            ImGui::Begin("Debug", nullptr,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetWindowSize(ImVec2(0.0, 0.0f));

            ImGui::TextUnformatted("Enter 'D1212' as a cheat-code to de/activate debug menu.");

            static bool bPacketTimeline = false;
            ImGui::Checkbox("Packet Timeline", &bPacketTimeline);
            if (bPacketTimeline)
            {
                CPacketTimeline::DrawUI();
            }

            static bool bMissionRunner = false;
            ImGui::Checkbox("Missions", &bMissionRunner);
            if (bMissionRunner && MissionRunner::DrawUI())
            {
                bMissionRunner = false;
                CImGui::ms_bDebugMenu = false;
                CImGui::UpdateActive();
            }

            ImGui::End();
            }
            ImGui::PopFont();

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

            ImGui_ImplDX9_InvalidateDeviceObjects();
        }
    };

    Events::gameProcessEvent.after += []
    {
        const char ACTIVATE_DEBUG_CHEAT[] = "2121D";  // type `d1212` as a cheat code
        if (strncmp(ACTIVATE_DEBUG_CHEAT, CCheat::m_CheatString, ARRAY_SIZE(ACTIVATE_DEBUG_CHEAT) - 1) == 0)
        {
            CCheat::m_CheatString[0] = '\0';
            CImGui::ms_bDebugMenu = !CImGui::ms_bDebugMenu;
            CImGui::UpdateActive();
        }

        // F7: coop menu (only when the game window has focus and the chat is closed)
        static bool bF7WasDown = false;
        bool bF7Down = (GetAsyncKeyState(VK_F7) & 0x8000) != 0;
        if (bF7Down && !bF7WasDown && GetForegroundWindow() == RsGlobal.ps->window && !CChat::m_bInputActive &&
            !FrontEndMenuManager.m_bMenuActive)
        {
            CImGui::ms_bCoopMenu = !CImGui::ms_bCoopMenu;
            CImGui::UpdateActive();
        }
        bF7WasDown = bF7Down;
    };
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CImGui::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}
