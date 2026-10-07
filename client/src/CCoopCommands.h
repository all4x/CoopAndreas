#pragma once

// Chat commands for free-roam coop (open the chat with F6).
// Commands marked "both players" are executed locally and then sent as a
// normal chat message; the other client recognizes them and executes them too.
class CCoopCommands
{
public:
    enum class eResult
    {
        NOT_A_COMMAND,  // regular chat message
        LOCAL_ONLY,     // handled, do not send
        BROADCAST,      // handled, send to the other players so they run it too
    };

    // typed by the local player
    static eResult HandleLocal(const std::wstring& text);

    // received from another player; returns true if it was a coop command
    static bool HandleRemote(const char* senderName, const wchar_t* text);
};
