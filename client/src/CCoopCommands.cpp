#include "stdafx.h"
#include "CCoopCommands.h"
#include "CCoopTeleport.h"
#include <CCheat.h>
#include <CModelInfo.h>
#include <cwctype>

namespace
{
enum class eCmd
{
    NONE,
    HELP,
    TP,
    BRING,
    HEAL,
    WEAPONS,
    NO_COPS,
    NEVER_WANTED,
    SKILLS,
    JETPACK,
    PARACHUTE,
    CAR,
};

struct SParsed
{
    eCmd cmd = eCmd::NONE;
    std::string arg;
};

std::string ToLowerAscii(const std::wstring& w)
{
    std::string s;
    s.reserve(w.size());
    for (wchar_t c : w)
        s.push_back(c < 128 ? (char)towlower(c) : '?');
    return s;
}

SParsed Parse(const std::wstring& text)
{
    SParsed p;
    std::string s = ToLowerAscii(text);
    if (s.empty() || s[0] != '/')
        return p;

    size_t space = s.find(' ');
    std::string name = s.substr(1, space == std::string::npos ? std::string::npos : space - 1);
    if (space != std::string::npos)
    {
        p.arg = s.substr(space + 1);
        while (!p.arg.empty() && p.arg.front() == ' ') p.arg.erase(p.arg.begin());
        while (!p.arg.empty() && p.arg.back() == ' ') p.arg.pop_back();
    }

    if (name == "ajuda" || name == "help") p.cmd = eCmd::HELP;
    else if (name == "tp") p.cmd = eCmd::TP;
    else if (name == "trazer") p.cmd = eCmd::BRING;
    else if (name == "vida") p.cmd = eCmd::HEAL;
    else if (name == "armas") p.cmd = eCmd::WEAPONS;
    else if (name == "policia") p.cmd = eCmd::NO_COPS;
    else if (name == "sempolicia") p.cmd = eCmd::NEVER_WANTED;
    else if (name == "habilidades") p.cmd = eCmd::SKILLS;
    else if (name == "jetpack") p.cmd = eCmd::JETPACK;
    else if (name == "paraquedas") p.cmd = eCmd::PARACHUTE;
    else if (name == "carro") p.cmd = eCmd::CAR;
    return p;
}

bool LocalPlayerReady()
{
    CPlayerPed* ped = FindPlayerPed(0);
    return ped && ped->IsAlive();
}

// effects that are applied on every player's own game
void ApplyShared(eCmd cmd, const std::string& arg)
{
    if (!LocalPlayerReady())
        return;

    switch (cmd)
    {
        case eCmd::HEAL:
            CCheat::MoneyArmourHealthCheat();  // HESOYAM: health, armour, $250k
            break;
        case eCmd::WEAPONS:
            if (arg == "1") CCheat::WeaponCheat1();
            else if (arg == "3") CCheat::WeaponCheat3();
            else CCheat::WeaponCheat2();
            break;
        case eCmd::NO_COPS:
            CCheat::WantedLevelDownCheat();  // ASNAEB
            break;
        case eCmd::NEVER_WANTED:
            CCheat::NotWantedCheat();  // AEZAKMI (toggle)
            break;
        case eCmd::SKILLS:
            CCheat::WeaponSkillsCheat();
            CCheat::VehicleSkillsCheat();
            break;
        default:
            break;
    }
}

void ShowHelp()
{
    CChat::AddMessage("{cecedb}[Coop] Comandos (abra o chat com F6):");
    CChat::AddMessage("{cecedb} /tp - ir ate o outro  |  /trazer - puxar o outro ate voce");
    CChat::AddMessage("{cecedb} /vida - vida+colete+dinheiro (os dois)  |  /policia - zera estrelas (os dois)");
    CChat::AddMessage("{cecedb} /armas [1|2|3] - pacote de armas (os dois)  |  /sempolicia - liga/desliga (os dois)");
    CChat::AddMessage("{cecedb} /habilidades - armas e direcao no maximo (os dois)");
    CChat::AddMessage("{cecedb} /carro <nome> (ex: /carro infernus)  |  /jetpack  |  /paraquedas");
}

void SpawnCar(const std::string& name)
{
    if (name.empty())
    {
        CChat::AddMessage("{cecedb}[Coop] Use: /carro <nome>  (ex: infernus, nrg500, sanchez, hydra, rhino)");
        return;
    }

    char buf[32];
    strncpy_s(buf, name.c_str(), sizeof(buf) - 1);
    int index = -1;
    CModelInfo::GetModelInfo(buf, &index);
    if (index < 400 || index > 611)
    {
        CChat::AddMessage("{cecedb}[Coop] Veiculo '%s' nao encontrado.", name.c_str());
        return;
    }

    CVehicle* vehicle = CCheat::VehicleCheat(index);
    if (vehicle)
    {
        vehicle->m_nVehicleFlags.bHasBeenOwnedByPlayer = true;
        logger::info("[coop-cmd] spawned vehicle %s (%d)", name.c_str(), index);
    }
    else
    {
        CChat::AddMessage("{cecedb}[Coop] Nao foi possivel criar '%s' agora.", name.c_str());
    }
}

bool IsShared(eCmd cmd)
{
    return cmd == eCmd::HEAL || cmd == eCmd::WEAPONS || cmd == eCmd::NO_COPS || cmd == eCmd::NEVER_WANTED ||
           cmd == eCmd::SKILLS;
}
}  // namespace

CCoopCommands::eResult CCoopCommands::HandleLocal(const std::wstring& text)
{
    SParsed p = Parse(text);
    if (p.cmd == eCmd::NONE)
        return eResult::NOT_A_COMMAND;

    logger::info("[coop-cmd] local %s", ToLowerAscii(text).c_str());

    switch (p.cmd)
    {
        case eCmd::HELP:
            ShowHelp();
            return eResult::LOCAL_ONLY;
        case eCmd::TP:
            CCoopTeleport::TeleportToPartnerCommand();
            return eResult::LOCAL_ONLY;
        case eCmd::BRING:
            CChat::AddMessage("{cecedb}[Coop] Chamando o outro jogador ate voce...");
            return eResult::BROADCAST;
        case eCmd::JETPACK:
            if (LocalPlayerReady()) CCheat::JetpackCheat();
            return eResult::LOCAL_ONLY;
        case eCmd::PARACHUTE:
            if (LocalPlayerReady()) CCheat::ParachuteCheat();
            return eResult::LOCAL_ONLY;
        case eCmd::CAR:
            SpawnCar(p.arg);
            return eResult::LOCAL_ONLY;
        default:
            break;
    }

    if (IsShared(p.cmd))
    {
        ApplyShared(p.cmd, p.arg);
        CChat::AddMessage("{cecedb}[Coop] %s aplicado para os dois.", ToLowerAscii(text).c_str());
        return eResult::BROADCAST;
    }

    return eResult::LOCAL_ONLY;
}

bool CCoopCommands::HandleRemote(const char* senderName, const wchar_t* text)
{
    SParsed p = Parse(text);
    if (p.cmd == eCmd::NONE)
        return false;

    logger::info("[coop-cmd] from %s: %s", senderName, ToLowerAscii(text).c_str());

    if (p.cmd == eCmd::BRING)
    {
        CChat::AddMessage("{cecedb}[Coop] %s puxou voce.", senderName);
        CCoopTeleport::TeleportToPartnerCommand();
        return true;
    }

    if (IsShared(p.cmd))
    {
        ApplyShared(p.cmd, p.arg);
        CChat::AddMessage("{cecedb}[Coop] %s usou %s.", senderName, ToLowerAscii(text).c_str());
    }
    // other commands are local to the sender: just don't show them as chat
    return true;
}
