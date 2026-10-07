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

struct SVehicleCategory
{
    const char* label;
    const SVehicleEntry* items;
    int count;
};

// model names checked against data/vehicles.ide of the 1.0 US install
const SVehicleEntry kCars[] = {
    {"Infernus", L"infernus"}, {"Turismo", L"turismo"},   {"Cheetah", L"cheetah"},   {"Bullet", L"bullet"},
    {"Super GT", L"supergt"},  {"Banshee", L"banshee"},   {"Sultan", L"sultan"},     {"Elegy", L"elegy"},
    {"Jester", L"jester"},     {"ZR-350", L"zr350"},      {"Comet", L"comet"},       {"Phoenix", L"phoenix"},
    {"Buffalo", L"buffalo"},   {"Hotring", L"hotring"},   {"Hotknife", L"hotknife"}, {"Savanna", L"savanna"},
    {"Blade", L"blade"},       {"Tornado", L"tornado"},   {"Slamvan", L"slamvan"},   {"Huntley", L"huntley"},
    {"Landstalker", L"landstal"}, {"Mesa", L"mesa"},      {"Sandking", L"sandking"}, {"Patriot", L"patriot"},
    {"Stretch (limusine)", L"stretch"}, {"Bandito", L"bandito"}, {"Kart", L"kart"},  {"Journey (trailer)", L"journey"},
    {"Onibus", L"bus"},        {"Bombeiro", L"firetruk"}, {"Policia", L"copcarla"},  {"SWAT", L"swatvan"},
    {"Barracks", L"barracks"}, {"Rhino (tanque)", L"rhino"},
};
const SVehicleEntry kOffroad[] = {
    {"Monster", L"monster"}, {"Monster A", L"monstera"}, {"Monster B", L"monsterb"}, {"Dune", L"duneride"},
    {"Dumper", L"dumper"},   {"Quadriciclo", L"quad"},
};
const SVehicleEntry kBikes[] = {
    {"NRG-500", L"nrg500"}, {"FCR-900", L"fcr900"}, {"PCJ-600", L"pcj600"}, {"BF-400", L"bf400"},
    {"Sanchez", L"sanchez"}, {"Freeway", L"freeway"}, {"Faggio", L"faggio"}, {"Moto policia", L"copbike"},
    {"Pizzaboy", L"pizzaboy"},
};
const SVehicleEntry kHelis[] = {
    {"Maverick", L"maverick"}, {"Maverick Policia", L"polmav"}, {"Maverick Noticias", L"vcnmav"},
    {"Sparrow", L"sparrow"},   {"Sea Sparrow", L"seaspar"},     {"Hunter (ataque)", L"hunter"},
    {"Cargobob", L"cargobob"}, {"Leviathan", L"leviathn"},      {"Raindance", L"raindanc"},
};
const SVehicleEntry kPlanes[] = {
    {"Shamal (jato)", L"shamal"}, {"Hydra (jato militar)", L"hydra"}, {"Rustler", L"rustler"},
    {"Stuntplane", L"stunt"},     {"Beagle", L"beagle"},              {"Cropduster", L"cropdust"},
    {"Nevada", L"nevada"},        {"Skimmer (hidroaviao)", L"skimmer"}, {"Vortex (hovercraft)", L"vortex"},
};
const SVehicleEntry kBoats[] = {
    {"Speeder", L"speeder"}, {"Squalo", L"squalo"}, {"Jetmax", L"jetmax"}, {"Dinghy", L"dinghy"},
    {"Marquis", L"marquis"}, {"Predator", L"predator"}, {"Reefer", L"reefer"}, {"Tropic", L"tropic"},
    {"Guarda costeira", L"coastg"}, {"Launch", L"launch"},
};

#define CATEGORY(label, arr) {label, arr, (int)(sizeof(arr) / sizeof(arr[0]))}
const SVehicleCategory kCategories[] = {
    CATEGORY("Carros", kCars),        CATEGORY("Off-road", kOffroad),
    CATEGORY("Motos", kBikes),        CATEGORY("Helicopteros", kHelis),
    CATEGORY("Avioes e jatos", kPlanes), CATEGORY("Barcos", kBoats),
};
#undef CATEGORY


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

    ImGui::Separator();
    ImGui::TextDisabled("Veiculos (aparecem do seu lado)");
    const int nCategories = (int)(sizeof(kCategories) / sizeof(kCategories[0]));
    for (int c = 0; c < nCategories; c++)
    {
        const SVehicleCategory& cat = kCategories[c];
        if (ImGui::CollapsingHeader(cat.label))
        {
            for (int i = 0; i < cat.count; i++)
            {
                if (i % 2 != 0)
                    ImGui::SameLine();
                ImGui::PushID(c * 100 + i);
                if (ImGui::Button(cat.items[i].label, ImVec2(127.0f, 0.0f)))
                {
                    CCoopCommands::Run(std::wstring(L"/carro ") + cat.items[i].model);
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::TextDisabled("Dica: aviao/heli precisa de espaco aberto ao redor.");

    ImGui::Separator();
    if (ImGui::Button("Fechar", ImVec2(w, 0.0f)))
        bOpen = false;

    ImGui::End();
    return bOpen;
}
