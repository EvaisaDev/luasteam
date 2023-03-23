#include "matchmaking.hpp"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
// ============================
// ======= SteamMatchmaking =======
// ============================

using luasteam::CallResultListener;

namespace {
class CallbackListener;
CallbackListener *matchmaking_listener = nullptr;

int matchmaking_ref = LUA_NOREF;

class CallbackListener {
  private:
    STEAM_CALLBACK(CallbackListener, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(CallbackListener, OnGameLobbyJoinRequested, GameLobbyJoinRequested_t);
    STEAM_CALLBACK(CallbackListener, OnLobbyChatMsgReceived, LobbyChatMsg_t);
    STEAM_CALLBACK(CallbackListener, OnLobbyDataUpdate, LobbyDataUpdate_t);
    STEAM_CALLBACK(CallbackListener, OnLobbyChatUpdate, LobbyChatUpdate_t);
};

void CallbackListener::OnGameLobbyJoinRequested(GameLobbyJoinRequested_t *data) {
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
    if (!lua_checkstack(L, 4)) {
        return;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, matchmaking_ref);
    lua_getfield(L, -1, "onGameLobbyJoinRequested");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
    } else {
        lua_createtable(L, 0, 2);
        luasteam::pushuint64(L, data->m_steamIDLobby.ConvertToUint64());
        lua_setfield(L, -2, "lobbyID");
        luasteam::pushuint64(L, data->m_steamIDFriend.ConvertToUint64());
        lua_setfield(L, -2, "friendID");
        //lua_call(L, 1, 0);
        if(lua_pcall(L, 1, 0, 0)) 
            lua_pop(L, 1);
        lua_pop(L, 1);
    }
}

void CallbackListener::OnLobbyEnter(LobbyEnter_t *data) {
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
    if (!lua_checkstack(L, 4)) {
        return;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, matchmaking_ref);
    lua_getfield(L, -1, "onLobbyEnter");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
    } else {
        lua_createtable(L, 0, 4);
        luasteam::pushuint64(L, data->m_ulSteamIDLobby);
        lua_setfield(L, -2, "lobbyID");
        lua_pushnumber(L, static_cast<int>(data->m_rgfChatPermissions));
        lua_setfield(L, -2, "permissions");
        lua_pushboolean(L, data->m_bLocked);
        lua_setfield(L, -2, "locked");
        lua_pushnumber(L, data->m_EChatRoomEnterResponse);
        lua_setfield(L, -2, "response");
        //lua_call(L, 1, 0);
        if(lua_pcall(L, 1, 0, 0)) 
            lua_pop(L, 1);
        lua_pop(L, 1);
    }
}

std::string convertToString(char* a, int size)
{
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
        // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
    }
    return elems;
}

void CallbackListener::OnLobbyChatMsgReceived(LobbyChatMsg_t *data) {
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
    if (!lua_checkstack(L, 4)) {
        return;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, matchmaking_ref);
    lua_getglobal(L, "print_error");
    lua_getfield(L, -2, "onLobbyChatMsgReceived");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
    } else {

        uint64 owner = SteamMatchmaking()->GetLobbyOwner(data->m_ulSteamIDLobby).ConvertToUint64();
        uint64 self = SteamUser()->GetSteamID().ConvertToUint64();

        char pvData[2048];

        int result = SteamMatchmaking()->GetLobbyChatEntry(data->m_ulSteamIDLobby, data -> m_iChatID, nullptr, &pvData, sizeof(pvData), nullptr);

        std::string pvDataStr = convertToString(pvData, sizeof(pvData));
        std::vector<std::string> split_pvData = split(pvDataStr, ';');

        if( owner == data->m_ulSteamIDUser){
            if (split_pvData.size() == 3) {
                if (split_pvData[0] == "kick") {
                    uint64 userID = std::stoull(split_pvData[1]);
                    if (userID == self) {
                        SteamMatchmaking()->LeaveLobby(data->m_ulSteamIDLobby);
                    }
                }   
            }
        }

        lua_createtable(L, 0, 5);
        luasteam::pushuint64(L, data->m_ulSteamIDLobby);
        lua_setfield(L, -2, "lobbyID");
        luasteam::pushuint64(L, data->m_ulSteamIDUser);
        lua_setfield(L, -2, "userID");
        lua_pushnumber(L, data->m_eChatEntryType);
        lua_setfield(L, -2, "type");
        lua_pushnumber(L, data -> m_iChatID);
        lua_setfield(L, -2, "chatID");
        lua_pushboolean(L, owner == data->m_ulSteamIDUser);
        lua_setfield(L, -2, "fromOwner");
        lua_pushstring(L, pvData);
        lua_setfield(L, -2, "message");
        if (lua_pcall(L, 1, 0, -3))
            lua_pop(L, 1); // Pop error string
        
        lua_pop(L, 1);
    }
}

void CallbackListener::OnLobbyDataUpdate(LobbyDataUpdate_t *data) {
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
    if (!lua_checkstack(L, 4)) {
        return;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, matchmaking_ref);
    lua_getfield(L, -1, "onLobbyDataUpdate");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
    } else {
        lua_createtable(L, 0, 3);
        luasteam::pushuint64(L, data->m_ulSteamIDLobby);
        lua_setfield(L, -2, "lobbyID");
        luasteam::pushuint64(L, data->m_ulSteamIDMember);
        lua_setfield(L, -2, "userID");
        lua_pushboolean(L, data->m_bSuccess);
        lua_setfield(L, -2, "success");
        //lua_call(L, 1, 0);
        if(lua_pcall(L, 1, 0, 0)) 
            lua_pop(L, 1);
        lua_pop(L, 1);
    }
}

void CallbackListener::OnLobbyChatUpdate(LobbyChatUpdate_t *data) {
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
    if (!lua_checkstack(L, 4)) {
        return;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, matchmaking_ref);
    lua_getfield(L, -1, "onLobbyChatUpdate");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
    } else {
        lua_createtable(L, 0, 4);
        luasteam::pushuint64(L, data->m_ulSteamIDLobby);
        lua_setfield(L, -2, "lobbyID");
        luasteam::pushuint64(L, data->m_ulSteamIDUserChanged);
        lua_setfield(L, -2, "userChanged");
        luasteam::pushuint64(L, data->m_ulSteamIDUserChanged);
        lua_setfield(L, -2, "userMakingChange");
        lua_pushnumber(L, data->m_rgfChatMemberStateChange);
        lua_setfield(L, -2, "chatMemberStateChange");
        //lua_call(L, 1, 0);
        if(lua_pcall(L, 1, 0, 0)) 
            lua_pop(L, 1);
        lua_pop(L, 1);
    }
}

const char *lobby_types[] = {"Private", "FriendsOnly", "Public", nullptr};
const char *lobby_comparisons[] = {"EqualToOrLessThan", "LessThan", "Equal", "GreaterThan", "EqualToOrGreaterThan", "NotEqual", nullptr};
const char *lobby_distance_filters[] = {"Close", "Default", "Far", "Worldwide", nullptr};

} // namespace

namespace luasteam {
    template <> void CallResultListener<LobbyCreated_t>::Result(LobbyCreated_t *data, bool io_fail) {
        lua_State *L = luasteam::global_lua_state;

        // getting stored lobby type
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref2);
        luaL_unref(L, LUA_REGISTRYINDEX, callback_ref2);

        const char *lobby_type = lua_tostring(L, -1);
        SteamMatchmaking()->SetLobbyData(data->m_ulSteamIDLobby, "LobbyType", lobby_type);
        SteamMatchmaking()->SetLobbyData(data->m_ulSteamIDLobby, "NewSystem", "True");

        // getting stored callback function
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);
        luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);


        //
        
        // calling function
        if (io_fail) {
            lua_pushnil(L);
        } else {
            lua_createtable(L, 0, 2);
            luasteam::pushuint64(L, data->m_eResult);
            lua_setfield(L, -2, "result");
            luasteam::pushuint64(L, data->m_ulSteamIDLobby);
            lua_setfield(L, -2, "lobby");
        }
        lua_pushboolean(L, io_fail);
        //lua_call(L, 2, 0);
        if(lua_pcall(L, 2, 0, 0)) 
            lua_pop(L, 1);

        delete this; // DELET THIS
    }

    template <> void CallResultListener<LobbyEnter_t>::Result(LobbyEnter_t *data, bool io_fail) {
        lua_State *L = luasteam::global_lua_state;

        // getting stored callback function
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);
        luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
        // calling function
        if (io_fail) {
            lua_pushnil(L);
        } else {
            lua_createtable(L, 0, 4);
            luasteam::pushuint64(L, data->m_ulSteamIDLobby);
            lua_setfield(L, -2, "lobbyID");
            lua_pushnumber(L, static_cast<int>(data->m_rgfChatPermissions));
            lua_setfield(L, -2, "permissions");
            lua_pushboolean(L, data->m_bLocked);
            lua_setfield(L, -2, "locked");
            lua_pushnumber(L, data->m_EChatRoomEnterResponse);
            lua_setfield(L, -2, "response");
        }
        lua_pushboolean(L, io_fail);
        //lua_call(L, 2, 0);
        if(lua_pcall(L, 2, 0, 0)) 
            lua_pop(L, 1);
        delete this; // DELET THIS
    }

    /*
        struct LobbyMatchList_t
        {
            enum { k_iCallback = k_iSteamMatchmakingCallbacks + 10 };
            uint32 m_nLobbiesMatching;		// Number of lobbies that matched search criteria and we have SteamIDs for
        };

    */

    template <> void CallResultListener<LobbyMatchList_t>::Result(LobbyMatchList_t *data, bool io_fail){
        lua_State *L = luasteam::global_lua_state;
        // getting stored callback function
        lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);
        luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
        // calling function
        if (io_fail) {
            lua_pushnil(L);
        } else {
            lua_createtable(L, 0, 1);
            lua_pushnumber(L, data->m_nLobbiesMatching);
            lua_setfield(L, -2, "count");
        }
        lua_pushboolean(L, io_fail);
        //lua_call(L, 2, 0);
        if(lua_pcall(L, 2, 0, 0)) 
            lua_pop(L, 1);
        delete this; // DELET THIS
    }

}

/*
EXTERN int luasteam_activateGameOverlay(lua_State *L) {
    const char *dialog = dialog_types[luaL_checkoption(L, 1, NULL, dialog_types)];
    SteamFriends()->ActivateGameOverlay(dialog);
    return 0;
}
*/



EXTERN int luasteam_createLobby(lua_State *L) {
    int eLobbyType = luaL_checkoption(L, 1, nullptr, lobby_types);
    const char *lobbyType = lobby_types[eLobbyType];

    auto *listener = new CallResultListener<LobbyCreated_t>();

    int cMaxMembers = luaL_checkint(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    listener->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushstring(L, lobbyType);
    listener->callback_ref2 = luaL_ref(L, LUA_REGISTRYINDEX);
    
    SteamAPICall_t call = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>(eLobbyType), cMaxMembers);
    listener->call_result.Set(call, listener, &CallResultListener<LobbyCreated_t>::Result);

    return 0;
}

EXTERN int luasteam_joinLobby(lua_State *L) {
    uint64_t lobbyID = luasteam::checkuint64(L, 1);
    //if(luasteam::steam_id_valid(lobbyID)){
        luaL_checktype(L, 2, LUA_TFUNCTION);
        auto *listener = new CallResultListener<LobbyEnter_t>();
        listener->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyID);
        listener->call_result.Set(call, listener, &CallResultListener<LobbyEnter_t>::Result);
    //}
    return 0;
}


EXTERN int luasteam_requestLobbyList(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    auto *listener = new CallResultListener<LobbyMatchList_t>();
    listener->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();
    listener->call_result.Set(call, listener, &CallResultListener<LobbyMatchList_t>::Result);
    return 0;
}

EXTERN int luasteam_getLobbyByIndex(lua_State *L) {
    int iLobby = luaL_checkint(L, 1);
    CSteamID lobbyID = SteamMatchmaking()->GetLobbyByIndex(iLobby);
    uint64 id = lobbyID.ConvertToUint64();

    // to string
    lua_createtable(L, 0, 1);
    luasteam::pushuint64(L, id);
    lua_setfield(L, -2, "lobbyID");
    
    return 1;
}

// InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee )
EXTERN int luasteam_inviteUserToLobby(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->InviteUserToLobby(luasteam::checkuint64(L, 1), luasteam::checkuint64(L, 2)));

    return 1;
}

EXTERN int luasteam_getNumLobbyMembers(lua_State *L) {
    lua_pushnumber(L, SteamMatchmaking()->GetNumLobbyMembers(luasteam::checkuint64(L, 1)));
    return 1;
}

EXTERN int luasteam_getLobbyMemberByIndex(lua_State *L) {
    luasteam::pushuint64(L, SteamMatchmaking()->GetLobbyMemberByIndex(luasteam::checkuint64(L, 1), luaL_checkint(L, 2)).ConvertToUint64());
    return 1;
}

EXTERN int luasteam_leaveLobby(lua_State *L) {
    SteamMatchmaking()->LeaveLobby(luasteam::checkuint64(L, 1));
    return 0;
}

EXTERN int luasteam_getLobbyOwner(lua_State *L) {
    luasteam::pushuint64(L, SteamMatchmaking()->GetLobbyOwner(luasteam::checkuint64(L, 1)).ConvertToUint64());
    return 1;
}

EXTERN int luasteam_setLobbyOwner(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->SetLobbyOwner(luasteam::checkuint64(L, 1), luasteam::checkuint64(L, 2)));
    return 1;
}

EXTERN int luasteam_setLobbyJoinable(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->SetLobbyJoinable(luasteam::checkuint64(L, 1), lua_toboolean(L, 2)));
    return 1;
}

EXTERN int luasteam_getLobbyMemberLimit(lua_State *L) {
    lua_pushnumber(L, SteamMatchmaking()->GetLobbyMemberLimit(luasteam::checkuint64(L, 1)));
    return 1;
}

EXTERN int luasteam_setLobbyMemberLimit(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->SetLobbyMemberLimit(luasteam::checkuint64(L, 1), luaL_checkint(L, 2)));
    return 1;
}

EXTERN int luasteam_setLobbyType(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->SetLobbyType(luasteam::checkuint64(L, 1), static_cast<ELobbyType>(luaL_checkoption(L, 2, nullptr, lobby_types))));
    return 1;
}

EXTERN int luasteam_getLobbyData(lua_State *L) {
    const char *key = luaL_checkstring(L, 2);

    const char *value = SteamMatchmaking()->GetLobbyData(luasteam::checkuint64(L, 1), key);
    if (value[0] == '\0') {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, value);
    }	
    return 1;
}

EXTERN int luasteam_setLobbyData(lua_State *L) {

    bool success = SteamMatchmaking()->SetLobbyData(luasteam::checkuint64(L, 1), luaL_checkstring(L, 2), luaL_checkstring(L, 3));

    lua_pushboolean(L, success);


    return 1;
}

EXTERN int luasteam_setLobbyMemberData(lua_State *L) {
    const char *key = luaL_checkstring(L, 2);
    const char *value = luaL_checkstring(L, 3);
    SteamMatchmaking()->SetLobbyMemberData(luasteam::checkuint64(L, 1), key, value);
    return 1;
}

EXTERN int luasteam_getLobbyMemberData(lua_State *L) {
    const char *key = luaL_checkstring(L, 3);
    lua_pushstring(L, SteamMatchmaking()->GetLobbyMemberData(luasteam::checkuint64(L, 1), luasteam::checkuint64(L, 2), key));
    return 1;
}

EXTERN int luasteam_addRequestLobbyListStringFilter(lua_State *L) {
    const char *key = luaL_checkstring(L, 1);
    const char *value = luaL_checkstring(L, 2);
    ELobbyComparison comparison = static_cast<ELobbyComparison>(luaL_checkoption(L, 3, nullptr, lobby_comparisons));
    SteamMatchmaking()->AddRequestLobbyListStringFilter(key, value, comparison);
    return 1;
}

EXTERN int luasteam_addRequestLobbyListResultCountFilter( lua_State *L ) {
    SteamMatchmaking()->AddRequestLobbyListResultCountFilter( luaL_checkint( L, 1 ) );
    return 1;
}

EXTERN int luasteam_deleteLobbyData(lua_State *L) {
    SteamMatchmaking()->DeleteLobbyData(luasteam::checkuint64(L, 1), luaL_checkstring(L, 2));
    return 0;
}

EXTERN int luasteam_getLobbyDataCount(lua_State *L) {
    lua_pushnumber(L, SteamMatchmaking()->GetLobbyDataCount(luasteam::checkuint64(L, 1)));
    return 1;
}

EXTERN int luasteam_getLobbyDataByIndex(lua_State *L) {
    // bool GetLobbyDataByIndex( CSteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize );
    char *key = new char[255];
    char *value = new char[255];

    SteamMatchmaking()->GetLobbyDataByIndex(luasteam::checkuint64(L, 1), luaL_checkint(L, 2), key, 255, value, 255);  

    // add key and value to a table and send to lua
    lua_newtable(L);
    lua_pushstring(L, key);
    lua_setfield(L, -2, "key");
    lua_pushstring(L, value);
    lua_setfield(L, -2, "value");
    

    return 1;
}

EXTERN int luasteam_sendLobbyChatMsg(lua_State *L) {
    const char *message = luaL_checkstring(L, 2);
    lua_pushboolean(L, SteamMatchmaking()->SendLobbyChatMsg(luasteam::checkuint64(L, 1), message, strlen(message) + 1));
    return 1;
}

EXTERN int luasteam_kickUserFromLobby(lua_State *L) {
    CSteamID lobby = luasteam::checkuint64(L, 1);
    CSteamID user = luasteam::checkuint64(L, 2);
    const char *reason = luaL_checkstring(L, 3);

    uint64_t lobby_id = lobby.ConvertToUint64();
    uint64_t user_id = user.ConvertToUint64();

    // make a string out of these seperated by semicolons like "kick;lobby_id;user_id;reason"
    std::string kick_string = "disconnect;";
    kick_string += std::to_string(user_id);
    kick_string += ";";
    kick_string += reason;

    // send the string to the server
    lua_pushboolean(L, SteamMatchmaking()->SendLobbyChatMsg(lobby, kick_string.c_str(), kick_string.length() + 1));

    return 0;
}

EXTERN int luasteam_getLobbyChatEntry(lua_State *L) {
    // int GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType );

    char *pvData = new char[255];

    int result = SteamMatchmaking()->GetLobbyChatEntry(luasteam::checkuint64(L, 1), luaL_checkint(L, 2), NULL, &pvData, 255, NULL);
    
    // Add results to table and send to lua
    lua_newtable(L);
    lua_pushstring(L, pvData);
    lua_setfield(L, -2, "data");

    return 0;
}

EXTERN int luasteam_addRequestLobbyListDistanceFilter(lua_State *L) {
    SteamMatchmaking()->AddRequestLobbyListDistanceFilter(static_cast<ELobbyDistanceFilter>(luaL_checkoption(L, 1, nullptr, lobby_distance_filters)));
    return 1;
}

EXTERN int luasteam_requestLobbyData(lua_State *L) {
    lua_pushboolean(L, SteamMatchmaking()->RequestLobbyData(luasteam::checkuint64(L, 1)));
    return 1;
}

namespace luasteam {

void add_matchmaking(lua_State *L) {
    lua_createtable(L, 0, 25);


    add_func(L, "createLobby", luasteam_createLobby);
    add_func(L, "inviteUserToLobby", luasteam_inviteUserToLobby);
    add_func(L, "joinLobby", luasteam_joinLobby);
    add_func(L, "requestLobbyList", luasteam_requestLobbyList);
    add_func(L, "getLobbyByIndex", luasteam_getLobbyByIndex);
    add_func(L, "getNumLobbyMembers", luasteam_getNumLobbyMembers);
    add_func(L, "getLobbyMemberByIndex", luasteam_getLobbyMemberByIndex);
    add_func(L, "leaveLobby", luasteam_leaveLobby);
    add_func(L, "getLobbyOwner", luasteam_getLobbyOwner);
    add_func(L, "setLobbyOwner", luasteam_setLobbyOwner);
    add_func(L, "setLobbyJoinable", luasteam_setLobbyJoinable);
    add_func(L, "getLobbyMemberLimit", luasteam_getLobbyMemberLimit);
    add_func(L, "setLobbyMemberLimit", luasteam_setLobbyMemberLimit);
    add_func(L, "setLobbyType", luasteam_setLobbyType);
    add_func(L, "getLobbyData", luasteam_getLobbyData);
    add_func(L, "setLobbyData", luasteam_setLobbyData);
    add_func(L, "setLobbyMemberData", luasteam_setLobbyMemberData);
    add_func(L, "getLobbyMemberData", luasteam_getLobbyMemberData);
    add_func(L, "sendLobbyChatMsg", luasteam_sendLobbyChatMsg);
    add_func(L, "addRequestLobbyListStringFilter", luasteam_addRequestLobbyListStringFilter);
    add_func(L, "deleteLobbyData", luasteam_deleteLobbyData);
    add_func(L, "addRequestLobbyListResultCountFilter", luasteam_addRequestLobbyListResultCountFilter);
    add_func(L, "getLobbyDataCount", luasteam_getLobbyDataCount);
    add_func(L, "getLobbyDataByIndex", luasteam_getLobbyDataByIndex);
    add_func(L, "getLobbyChatEntry", luasteam_getLobbyChatEntry);
    add_func(L, "kickUserFromLobby", luasteam_kickUserFromLobby);
    add_func(L, "addRequestLobbyListDistanceFilter", luasteam_addRequestLobbyListDistanceFilter);
    add_func(L, "requestLobbyData", luasteam_requestLobbyData);

    lua_pushvalue(L, -1);
    matchmaking_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_setfield(L, -2, "matchmaking");
}

void init_matchmaking(lua_State *L) { matchmaking_listener = new CallbackListener(); }

void shutdown_matchmaking(lua_State *L) {
    luaL_unref(L, LUA_REGISTRYINDEX, matchmaking_ref);
    matchmaking_ref = LUA_NOREF;
    delete matchmaking_listener;
    matchmaking_listener = nullptr;
}

} // namespace luasteam
