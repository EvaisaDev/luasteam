#include "timeline.hpp"
#include <cstdlib>

/*
SetTimelineStateDescription
void SetTimelineStateDescription( const char *pchDescription, float flTimeDelta );
pchDescription	const char *	A localized string in the language returned by SteamUtils()->GetSteamUILanguage()
flTimeDelta	float	The time offset in seconds to apply to this state change. Negative times indicate an
event that happened in the past.

Sets a description (B) for the current game state in the timeline. These help the user to find specific
moments in the timeline when saving clips. Setting a new state description replaces any previous
description.

Examples could include:

    Where the user is in the world in a single player game
    Which round is happening in a multiplayer game
    The current score for a sports game
*/

EXTERN int luasteam_setTimelineStateDescription(lua_State *L) {
    const char *description = luaL_checkstring(L, 1);
    float timeDelta = luaL_checknumber(L, 2);

    SteamTimeline()->SetTimelineStateDescription(description, timeDelta);

    return 1;
}


/*
AddTimelineEvent
 void AddTimelineEvent( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip ) ;

pchIcon	const char *	The name of the icon to show at the timeline at this point. This can be one of the icons uploaded through the Steamworks partner Site for your title, or one of the provided icons that start with steam_. The Steam Timelines overview includes a list of available icons.
pchTitle	const char *	Title-provided localized string in the language returned by SteamUtils()->GetSteamUILanguage().
pchDescription	const char *	Title-provided localized string in the language returned by SteamUtils()->GetSteamUILanguage().
unPriority	uint32	Provide the priority to use when the UI is deciding which icons to display in crowded parts of the timeline. Events with larger priority values will be displayed more prominently than events with smaller priority values. This value must be between 0 and k_unMaxTimelinePriority.
flStartOffsetSeconds	float	The time offset in seconds to apply to the start of the event. Negative times indicate an
event that happened in the past.

One use of this parameter is to handle events whose significance is not clear until after the fact. For instance if the player starts a damage over time effect on another player, which kills them 3.5 seconds later, the game could pass -3.5 as the start offset and cause the event to appear in the timeline where the effect started.
flDurationSeconds	float	The duration of the event, in seconds. Pass 0 for instantaneous events.
ePossibleClip 	ETimelineEventClipPriority	Allows the game to describe events that should be suggested to the user as possible video clips.

Use this to mark an event (A) on the Timeline. The event can be instantaneous or take some amount of time to complete, depending on the value passed in flDurationSeconds.

Examples could include:

    a boss battle
    a cut scene
    a large team fight
    picking up a new weapon or ammo
    scoring a goal


The game can nominate an event as being suitable for a clip by passing k_ETimelineEventClipPriority_Standard or k_ETimelineEventClipPriority_Featured to ePossibleClip. Players can make clips of their own at any point, but this lets the game suggest some options to Steam to make that process easier for players.
*/

EXTERN int luasteam_addTimelineEvent(lua_State *L) {
    const char *icon = luaL_checkstring(L, 1);
    const char *title = luaL_checkstring(L, 2);
    const char *description = luaL_checkstring(L, 3);
    uint32 priority = luaL_checkint(L, 4);
    float startOffset = luaL_checknumber(L, 5);
    float duration = luaL_checknumber(L, 6);
    
    /*
    enum ETimelineEventClipPriority
    {
        k_ETimelineEventClipPriority_Invalid = 0,
        k_ETimelineEventClipPriority_None = 1,
        k_ETimelineEventClipPriority_Standard = 2,
        k_ETimelineEventClipPriority_Featured = 3,
    };
    */

    ETimelineEventClipPriority possibleClip = static_cast<ETimelineEventClipPriority>(luaL_checkint(L, 7));

    SteamTimeline()->AddTimelineEvent(icon, title, description, priority, startOffset, duration, possibleClip);

    return 1;
}

/*
SetTimelineGameMode
void SetTimelineGameMode( ETimelineGameMode eMode );

eMode	ETimelineGameMode	The mode that the game is in.

Changes the color of the timeline bar (C). See ETimelineGameMode for how to use each valu
*/

EXTERN int luasteam_setTimelineGameMode(lua_State *L) {
    ETimelineGameMode mode = static_cast<ETimelineGameMode>(luaL_checkint(L, 1));

    /*
    enum ETimelineGameMode
    {
        k_ETimelineGameMode_Invalid = 0,
        k_ETimelineGameMode_Playing = 1,
        k_ETimelineGameMode_Staging = 2,
        k_ETimelineGameMode_Menus = 3,
        k_ETimelineGameMode_LoadingScreen = 4,

        k_ETimelineGameMode_Max, // one past the last valid value
    };
    */

    SteamTimeline()->SetTimelineGameMode(mode);

    return 1;
}



namespace luasteam {

void add_timeline(lua_State *L) {
    lua_createtable(L, 0, 3);

    add_func(L, "setTimelineStateDescription", luasteam_setTimelineStateDescription);
    add_func(L, "addTimelineEvent", luasteam_addTimelineEvent);
    add_func(L, "setTimelineGameMode", luasteam_setTimelineGameMode);
    
    lua_setfield(L, -2, "timeline");
}

void init_timeline(lua_State *L) {}

void shutdown_timeline(lua_State *L) {}

} // namespace luasteam
