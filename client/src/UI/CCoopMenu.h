#pragma once

// Simple clickable coop menu (F7). Every button runs the same code as the
// matching chat command in CCoopCommands, so behavior stays in one place.
class CCoopMenu
{
public:
    // draws the window; returns false when the user closed it
    static bool Draw();
};
