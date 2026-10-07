#include "stdafx.h"
#include "CCoopMenu.h"
#include "CCoopCommands.h"
#include "imgui.h"

namespace
{
struct SVehicleEntry
{
    const char* label;
    const wchar_t* model;  // model name used by /carro
};

const SVehicleEntry kVehicles[] = {
    {"Infernus", L"infernus"},   {"Turismo", L"turismo"},   {"Cheetah", L"cheetah"},   {"Bullet", L"bullet"},
    {"Banshee", L"banshee"},     {"Sultan", L"sultan"},     {"Elegy", L"elegy"},       {"Sandking", L"sandking"},
    {"Patriot", L"patriot"},     {"Monster", L"monster"},   {"NRG-500", L"nrg500"},    {"PCJ-600", L"pcj600"},
    {"Sanchez", L"sanchez"},     {"BMX", L"bmx"},           {"Maverick", L"maverick"}, {"Hunter", L"hunter"},
    {"Hydra", L"hydra"},         {"Rhino (tanque)", L"rhino"},
};

void Button(const char* label, const wchar_t* command, float width = 0.0f)
{
    if (ImGui::Button(label, ImVec2(width, 0.0f)))
    {
        CCoopCommands::Run(command);
    }
}
}  // namespace

bool CCoopMenu::Draw()
{
    bool bOpen = true;

    ImGui::SetNextWindowPos(ImVec2(40.0f, 120.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Menu Coop  (F7 abre/fecha)", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

    const float w = 260.0f;

    ImGui::TextDisabled("Teleporte (fora de missao e de interiores)");
    Button("Ir ate o parceiro", L"/tp", w);
    Button("Trazer o parceiro ate mim", L"/trazer", w);

    ImGui::Separator();
    ImGui::TextDisabled("Para os dois jogadores");
    Button("Vida + colete + dinheiro", L"/vida", w);
    Button("Tirar policia (zerar estrelas)", L"/policia", w);
    Button("Nunca procurado (liga/desliga)", L"/sempolicia", w);
    Button("Armas 1", L"/armas 1", 82.0f);
    ImGui::SameLine();
    Button("Armas 2", L"/armas 2", 82.0f);
    ImGui::SameLine();
    Button("Armas 3", L"/armas 3", 82.0f);
    Button("Habilidades no maximo", L"/habilidades", w);

    ImGui::Separator();
    ImGui::TextDisabled("So para mim");
    Button("Jetpack", L"/jetpack", 127.0f);
    ImGui::SameLine();
    Button("Paraquedas", L"/paraquedas", 127.0f);

    static int s_selectedVehicle = 0;
    ImGui::SetNextItemWidth(160.0f);
    if (ImGui::BeginCombo("##veiculo", kVehicles[s_selectedVehicle].label))
    {
        for (int i = 0; i < (int)(sizeof(kVehicles) / sizeof(kVehicles[0])); i++)
        {
            bool bSelected = (i == s_selectedVehicle);
            if (ImGui::Selectable(kVehicles[i].label, bSelected))
                s_selectedVehicle = i;
            if (bSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Criar veiculo", ImVec2(94.0f, 0.0f)))
    {
        CCoopCommands::Run(std::wstring(L"/carro ") + kVehicles[s_selectedVehicle].model);
    }

    ImGui::Separator();
    if (ImGui::Button("Fechar", ImVec2(w, 0.0f)))
        bOpen = false;

    ImGui::End();
    return bOpen;
}
