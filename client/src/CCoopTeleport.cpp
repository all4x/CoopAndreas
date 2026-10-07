#include "stdafx.h"
#include "CCoopTeleport.h"
#include <CGeneral.h>
#include <CPedPlacement.h>
#include <CStreaming.h>

// time to wait after the respawn before teleporting, so the hospital/police
// restart (fade, position, clothes) has finished
static constexpr uint32_t RESPAWN_TELEPORT_DELAY_MS = 2500;

static bool IsOnMission()
{
    return CTheScripts::OnAMissionFlag && CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] != 0;
}

static CNetworkPlayer* FindPartner()
{
    for (auto* networkPlayer : CNetworkPlayerManager::m_pPlayers)
    {
        if (networkPlayer && networkPlayer->m_pPed && networkPlayer->m_pPed->IsAlive())
            return networkPlayer;
    }
    return nullptr;
}

// same idea as CCommandTeleportPlayersToHostSafely: a free spot around the target
static bool FindFreeSpotNear(const CVector& target, float heading, CVector& out)
{
    for (float dist = 1.5f; dist <= 6.0f; dist += 0.5f)
    {
        for (float angle = heading + 2.35619f; angle < heading + 6.28319f + 2.35619f; angle += 0.5f)
        {
            float rad = CGeneral::LimitRadianAngle(angle);
            CVector pos = target + CVector(-sinf(rad), cosf(rad), 0.0f) * dist;

            if (!CPedPlacement::FindZCoorForPed(&pos))
                continue;

            if (CWorld::TestSphereAgainstWorld(pos, 0.35f, nullptr, true, true, true, true, true, false))
                continue;

            out = pos;
            return true;
        }
    }
    return false;
}

bool CCoopTeleport::TeleportToPartner(bool bFromRespawn)
{
    const char* szWhy = bFromRespawn ? "respawn" : "/tp";

    CPlayerPed* pLocal = FindPlayerPed(0);
    if (!pLocal || !pLocal->IsAlive())
        return false;

    if (IsOnMission())
    {
        if (!bFromRespawn)
            CChat::AddMessage("{cecedb}[Coop] /tp desativado durante missao.");
        logger::info("[coop-tp] %s skipped: on mission", szWhy);
        return false;
    }

    CNetworkPlayer* pPartner = FindPartner();
    if (!pPartner)
    {
        if (!bFromRespawn)
            CChat::AddMessage("{cecedb}[Coop] Nenhum outro jogador vivo para teleportar.");
        return false;
    }

    if (pLocal->m_nPedFlags.bInVehicle)
    {
        if (!bFromRespawn)
            CChat::AddMessage("{cecedb}[Coop] Saia do veiculo para usar /tp.");
        return false;
    }

    CPlayerPed* pTarget = pPartner->m_pPed;
    if (pTarget->m_nAreaCode != 0 || CGame::currArea != 0)
    {
        if (!bFromRespawn)
            CChat::AddMessage("{cecedb}[Coop] /tp so funciona com os dois fora de interiores.");
        logger::info("[coop-tp] %s skipped: interior (partner area %d, local area %d)", szWhy,
            (int)pTarget->m_nAreaCode, (int)CGame::currArea);
        return false;
    }

    CVector target = pTarget->GetPosition();
    if (pTarget->m_nPedFlags.bInVehicle && pTarget->m_pVehicle)
        target = pTarget->m_pVehicle->GetPosition();

    // make sure the collision around the destination is loaded before searching for ground
    CStreaming::LoadSceneCollision(&target);
    CStreaming::LoadScene(&target);

    CVector dest;
    if (!FindFreeSpotNear(target, pTarget->m_fCurrentRotation, dest))
    {
        dest = target + CVector(1.5f, 1.5f, 1.0f);
    }

    pLocal->Teleport(dest, false);
    pLocal->m_fCurrentRotation = pTarget->m_fCurrentRotation;
    pLocal->m_fAimingRotation = pTarget->m_fCurrentRotation;
    pLocal->SetHeading(pTarget->m_fCurrentRotation);
    pLocal->UpdateRwMatrix();

    logger::info("[coop-tp] %s: teleported next to player %d '%s' (%.1f %.1f %.1f)", szWhy, pPartner->m_iPlayerId,
        pPartner->m_Name, dest.x, dest.y, dest.z);
    CChat::AddMessage("{cecedb}[Coop] Teleportado para perto de %s.", pPartner->m_Name);
    return true;
}

void CCoopTeleport::OnLocalRespawn()
{
    ms_nPendingRespawnTeleportAt = GetTickCount() + RESPAWN_TELEPORT_DELAY_MS;
}

void CCoopTeleport::Process()
{
    if (ms_nPendingRespawnTeleportAt == 0 || GetTickCount() < ms_nPendingRespawnTeleportAt)
        return;

    ms_nPendingRespawnTeleportAt = 0;
    TeleportToPartner(true);
}
