#include "networking.hpp"

// ============================
// ======= SteamNetworkingMessages =======
// ============================

//int networkingidentityMetatable_ref = LUA_NOREF;

void my_assert(lua_State *L, int cond, const char *fmt, va_list list) {
    if (cond) {
        return;
    }
    lua_pushvfstring(L, fmt, list);
    lua_error(L);
}
/*
void pushnetworkingidentity(lua_State *L, SteamNetworkingIdentity v) {
	SteamNetworkingIdentity *ptr = reinterpret_cast<SteamNetworkingIdentity *>(lua_newuserdata(L, sizeof(SteamNetworkingIdentity)));
	*ptr = v;
	lua_rawgeti(L, LUA_REGISTRYINDEX, networkingidentityMetatable_ref);
	lua_setmetatable(L, -2);
}

SteamNetworkingIdentity checknetworkingidentity(lua_State *L, int nParam) {
	luaL_argcheck(L, lua_isuserdata(L, nParam), nParam, "must be userdata");
	lua_rawgeti(L, LUA_REGISTRYINDEX, networkingidentityMetatable_ref);
	lua_getmetatable(L, nParam);
	luaL_argcheck(L, lua_rawequal(L, -2, -1), nParam, "must be SteamNetworkingIdentity");
	lua_pop(L, 2);
	SteamNetworkingIdentity *ptr = reinterpret_cast<SteamNetworkingIdentity *>(lua_touserdata(L, nParam));
	return *ptr;
}

SteamNetworkingIdentity assertnetworkingidentity(lua_State *L, int index, const char *fmt, ...) {
	va_list list;
	va_start(list, fmt);
	my_assert(L, lua_isuserdata(L, index), fmt, list);
	lua_rawgeti(L, LUA_REGISTRYINDEX, networkingidentityMetatable_ref);
	lua_getmetatable(L, index);
	my_assert(L, lua_rawequal(L, -2, -1), fmt, list);
	lua_pop(L, 2);
	SteamNetworkingIdentity *ptr = reinterpret_cast<SteamNetworkingIdentity *>(lua_touserdata(L, index));
	return *ptr;
}
*/
namespace {

class CallbackListener;
CallbackListener *networking_listener = nullptr;
int networking_ref = LUA_NOREF;

class CallbackListener {
  private:
    STEAM_CALLBACK(CallbackListener, OnSteamNetworkingMessagesSessionRequest, SteamNetworkingMessagesSessionRequest_t);
	STEAM_CALLBACK(CallbackListener, OnSteamNetworkingMessagesSessionFailed, SteamNetworkingMessagesSessionFailed_t);
};

/*
// SteamNetworkingMessagesSessionRequest_t
struct SteamNetworkingMessagesSessionRequest_t
{ 
	/// User who wants to talk to us
	SteamNetworkingIdentity m_identityRemote;
};
*/
// return steam_id
void CallbackListener::OnSteamNetworkingMessagesSessionRequest(SteamNetworkingMessagesSessionRequest_t *data)
{
    if (data == nullptr) {
        return;
    }
    lua_State *L = luasteam::global_lua_state;
	lua_rawgeti(L, LUA_REGISTRYINDEX, networking_ref);
	lua_getfield(L, -1, "onSessionRequest");
	if (lua_isfunction(L, -1)) {
		luasteam::pushuint64(L, data->m_identityRemote.GetSteamID().ConvertToUint64());
		lua_pcall(L, 1, 0, 0);
	}
	lua_pop(L, 2);
}

// SteamNetworkingMessagesSessionFailed_t
/*
struct SteamNetworkingMessagesSessionFailed_t
{ 
	/// Detailed info about the session that failed.
	/// SteamNetConnectionInfo_t::m_identityRemote indicates who this session
	/// was with.
	SteamNetConnectionInfo_t m_info;
};
*/

void CallbackListener::OnSteamNetworkingMessagesSessionFailed(SteamNetworkingMessagesSessionFailed_t *data)
{
	if (data == nullptr) {
		return;
	}
	lua_State *L = luasteam::global_lua_state;
	lua_rawgeti(L, LUA_REGISTRYINDEX, networking_ref);
	lua_getfield(L, -1, "onSessionFailed");
	if (lua_isfunction(L, -1)) {
		luasteam::pushuint64(L, data->m_info.m_identityRemote.GetSteamID().ConvertToUint64());
		// push m_eEndReason int
		// push m_szEndDebug char*
		// push m_szConnectionDescription char*
		lua_pushinteger(L, data->m_info.m_eEndReason);
		lua_pushstring(L, data->m_info.m_szEndDebug);
		lua_pushstring(L, data->m_info.m_szConnectionDescription);

		// pcall
		lua_pcall(L, 4, 0, 0);
	}
	lua_pop(L, 2);
}




} // namespace

/*
void pushuint64(lua_State *L, uint64 v) {
    uint64 *ptr = reinterpret_cast<uint64 *>(lua_newuserdata(L, sizeof(uint64)));
    *ptr = v;
    lua_rawgeti(L, LUA_REGISTRYINDEX, uint64Metatable_ref);
    lua_setmetatable(L, -2);
}

uint64 checkuint64(lua_State *L, int nParam) {
    luaL_argcheck(L, lua_isuserdata(L, nParam), nParam, "must be userdata");
    lua_rawgeti(L, LUA_REGISTRYINDEX, uint64Metatable_ref);
    lua_getmetatable(L, nParam);
    luaL_argcheck(L, lua_rawequal(L, -2, -1), nParam, "must be uint64");
    lua_pop(L, 2);
    uint64 *ptr = reinterpret_cast<uint64 *>(lua_touserdata(L, nParam));
    return *ptr;
}

uint64 assertuint64(lua_State *L, int index, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    my_assert(L, lua_isuserdata(L, index), fmt, list);
    lua_rawgeti(L, LUA_REGISTRYINDEX, uint64Metatable_ref);
    lua_getmetatable(L, index);
    my_assert(L, lua_rawequal(L, -2, -1), fmt, list);
    lua_pop(L, 2);
    uint64 *ptr = reinterpret_cast<uint64 *>(lua_touserdata(L, index));
    re
*/

// add a function to find the SteamNetworkingIdentity of a user using their SteamID
// return SteamNetworkingIdentity
/*
EXTERN int luasteam_get_identity(lua_State *L) {
	CSteamID user = luasteam::checkuint64(L, 1);
	if(luasteam::steam_id_valid(user)){
		SteamNetworkingIdentity identity;
		identity.SetSteamID(user);
		pushnetworkingidentity(L, identity);
	}else{
		lua_pushnil(L);
	}
	return 1;
}
*/
// turn a SteamNetworkingIdentity into a SteamID
/*
EXTERN int luasteam_getsteamid(lua_State *L) {
	SteamNetworkingIdentity identity = checknetworkingidentity(L, 1);
	luasteam::pushuint64(L, identity.GetSteamID().ConvertToUint64());
	return 1;
}
*/

// Write a wrapper for SendMessageToUser
// EResult SendMessageToUser( const SteamNetworkingIdentity &identityRemote, const void *pubData, uint32 cubData, int nSendFlags, int nRemoteChannel );
// make it so you only have to give a steam_id and the message
// return EResult

EXTERN int luasteam_sendstring(lua_State *L) {
	CSteamID user = luasteam::checkuint64(L, 1);
	if(luasteam::steam_id_valid(user)){
		size_t len;
		const char *data = luaL_checklstring(L, 2, &len);
		SteamNetworkingIdentity identity;
		identity.SetSteamID(user);
		EResult result = SteamNetworkingMessages()->SendMessageToUser(identity, data, len, k_nSteamNetworkingSend_Reliable, 0);
		luasteam::pushuint64(L, result);
		// push message size
		lua_pushinteger(L, len);
	}else{
		luasteam::pushuint64(L, k_EResultInvalidSteamID);
	}
	return 2;
}

// Write a wrapper for AcceptSessionWithUser
// bool AcceptSessionWithUser( const SteamNetworkingIdentity &identityRemote );
// take a steam id and return a bool
EXTERN int luasteam_acceptsession(lua_State *L) {
	CSteamID user = luasteam::checkuint64(L, 1);
	// check if user is a valid steam id, use GetFriendPersonaName
	// first check if user is not 0
	if(luasteam::steam_id_valid(user)){

		SteamNetworkingIdentity identity;
		identity.SetSteamID(user);
		lua_pushboolean(L, SteamNetworkingMessages()->AcceptSessionWithUser(identity));
	}else{
		lua_pushboolean(L, false);
	}
	return 1;
}

EXTERN int luasteam_closesession(lua_State *L) {
	CSteamID user = luasteam::checkuint64(L, 1);
	if(luasteam::steam_id_valid(user)){
		SteamNetworkingIdentity identity;
		identity.SetSteamID(user);
		lua_pushboolean(L, SteamNetworkingMessages()->CloseSessionWithUser(identity));
	}else{
		lua_pushboolean(L, false);
	}
	return 1;
}

// Write a wrapper for ReceiveMessagesOnChannel
// int ReceiveMessagesOnChannel( int nLocalChannel, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages );
// don't take a channel input, channel is always 0
// return messages in a table, limit to 10 messages
// It should be in the following format {{data = [message], user = [steamid]}}
// When you're done with the message object(s), make sure and call SteamNetworkingMessage_t::Release!
EXTERN int luasteam_pollMessages(lua_State *L) {
    SteamNetworkingMessage_t *messages[10];
    int num = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, 10);
    lua_createtable(L, num, 0);
    for (int i = 0; i < num; i++) {
        lua_createtable(L, 0, 3);
        lua_pushstring(L, "data");
        lua_pushlstring(L, (const char *)messages[i]->m_pData, messages[i]->m_cbSize);
        lua_settable(L, -3);
        lua_pushstring(L, "user");
        luasteam::pushuint64(L, messages[i]->m_identityPeer.GetSteamID().ConvertToUint64());
        lua_settable(L, -3);
        lua_pushstring(L, "msg_size");
        lua_pushinteger(L, messages[i]->m_cbSize);
        lua_settable(L, -3);
        lua_rawseti(L, -2, i + 1);
        messages[i]->Release();
    }
    return 1;
}

// Wrapper for GetSessionConnectionInfo
// ESteamNetworkingConnectionState GetSessionConnectionInfo( const SteamNetworkingIdentity &identityRemote, SteamNetConnectionInfo_t *pConnectionInfo, SteamNetworkingQuickConnectionStatus *pQuickStatus );
// return whatever info is available in a table
/*
{
	result = ESteamNetworkingConnectionState,
	connectionInfo = {
		steamIDRemote = m_identityRemote.GetSteamID().ConvertToUint64(),
		userData = m_nUserData,
		eEndReason = m_eEndReason,
		szEndDebug = m_szEndDebug (as string),
		szConnectionDescription = m_szConnectionDescription (as string),
	},
	connectionStatus = {
		nPing = m_nPing,
	}
}
*/
EXTERN int luasteam_getConnectionInfo(lua_State *L) {
	CSteamID user = luasteam::checkuint64(L, 1);
	SteamNetworkingIdentity identity;
	identity.SetSteamID(user);
	SteamNetConnectionInfo_t info;
	SteamNetConnectionRealTimeStatus_t quick;
	ESteamNetworkingConnectionState result = SteamNetworkingMessages()->GetSessionConnectionInfo(identity, &info, &quick);
	lua_createtable(L, 0, 3);
	lua_pushstring(L, "result");
	lua_pushinteger(L, result);
	lua_settable(L, -3);
	lua_pushstring(L, "connectionInfo");
	lua_createtable(L, 0, 6);
	lua_pushstring(L, "steamIDRemote");
	luasteam::pushuint64(L, info.m_identityRemote.GetSteamID().ConvertToUint64());
	lua_settable(L, -3);
	lua_pushstring(L, "userData");
	lua_pushinteger(L, info.m_nUserData);
	lua_settable(L, -3);
	lua_pushstring(L, "eEndReason");
	lua_pushinteger(L, info.m_eEndReason);
	lua_settable(L, -3);
	lua_pushstring(L, "szEndDebug");
	lua_pushstring(L, info.m_szEndDebug);
	lua_settable(L, -3);
	lua_pushstring(L, "szConnectionDescription");
	lua_pushstring(L, info.m_szConnectionDescription);
	lua_settable(L, -3);
	lua_settable(L, -3);
	lua_pushstring(L, "connectionStatus");
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "nPing");
	lua_pushinteger(L, quick.m_nPing);
	lua_settable(L, -3);
	lua_settable(L, -3);
	return 1;
}

namespace luasteam {

void add_networking(lua_State *L) {
    lua_createtable(L, 0, 4);
	add_func(L, "acceptSession", luasteam_acceptsession);
	add_func(L, "closeSession", luasteam_closesession);
	add_func(L, "pollMessages", luasteam_pollMessages);
	//add_func(L, "getIdentity", luasteam_get_identity);
	//add_func(L, "getSteamID", luasteam_getsteamid);
	add_func(L, "sendString", luasteam_sendstring);
	add_func(L, "getConnectionInfo", luasteam_getConnectionInfo);

    lua_pushvalue(L, -1);
    networking_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_setfield(L, -2, "networking");
}

void init_networking(lua_State *L) { 
	// InitRelayNetworkAccess();
	//SteamNetworkingUtils()->InitRelayNetworkAccess();

	// SetConfigValue
	// k_ESteamNetworkingConfig_IP_AllowWithoutAuth
	//SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );


	networking_listener = new CallbackListener(); 
}

void shutdown_networking(lua_State *L) {
    luaL_unref(L, LUA_REGISTRYINDEX, networking_ref);
    networking_ref = LUA_NOREF;
    delete networking_listener;
    networking_listener = nullptr;
}

} // namespace luasteam
