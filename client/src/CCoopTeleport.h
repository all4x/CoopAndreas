#pragma once

// Quality-of-life teleports for free-roam coop:
//   - chat command "/tp": teleport next to the other player
//   - after death/arrest respawn: teleport back next to the other player
// Both are skipped during missions (the host's mission script owns player
// positions there) and when either player is inside an interior.
class CCoopTeleport
{
public:
    // returns true if `text` was a coop command and must not be sent as chat
    static bool HandleChatCommand(const std::wstring& text);

    // called when the local player has just been restarted (hospital/police)
    static void OnLocalRespawn();

    // called every frame while authenticated
    static void Process();

private:
    static bool TeleportToPartner(bool bFromRespawn);

    static inline uint32_t ms_nPendingRespawnTeleportAt = 0;
};
