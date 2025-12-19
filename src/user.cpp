#include <cstdlib> 
#include "user.hpp"

// int GetPlayerSteamLevel();
EXTERN int luasteam_getPlayerSteamLevel(lua_State *L) {
    lua_pushnumber(L, SteamUser()->GetPlayerSteamLevel());
    return 1;
}

// CSteamID GetSteamID();
EXTERN int luasteam_getSteamID(lua_State *L) {
    luasteam::pushuint64(L, SteamUser()->GetSteamID().ConvertToUint64());
    return 1;
}


EXTERN int luasteam_loggedOn(lua_State *L){
    lua_pushboolean(L, SteamUser()->BLoggedOn());
    return 1;
}

// void StartVoiceRecording();
EXTERN int luasteam_startVoiceRecording(lua_State *L) {
	SteamUser()->StartVoiceRecording();
	return 0;
}

// void StopVoiceRecording();
EXTERN int luasteam_stopVoiceRecording(lua_State *L) {
	SteamUser()->StopVoiceRecording();
	return 0;
}

// EVoiceResult GetAvailableVoice( uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated = 0, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated = 0 );
EXTERN int luasteam_getAvailableVoice(lua_State *L) {
	uint32 pcbCompressed;
	uint32 pcbUncompressed;
	EVoiceResult result = SteamUser()->GetAvailableVoice(&pcbCompressed, &pcbUncompressed);
	lua_pushnumber(L, pcbCompressed);
	lua_pushnumber(L, pcbUncompressed);
	lua_pushnumber(L, result);
	return 3;
}

// EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated = false, void *pUncompressedDestBuffer_Deprecated = 0, uint32 cbUncompressedDestBufferSize_Deprecated = 0, uint32 *nUncompressBytesWritten_Deprecated = 0, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated = 0 );
EXTERN int luasteam_getVoice(lua_State *L) {
	uint32 cbDestBufferSize = luaL_checknumber(L, 1);
	uint32 nBytesWritten;
	uint32 cbUncompressedDestBufferSize = 0;
	uint32 nUncompressBytesWritten = 0;
	uint32 nUncompressedVoiceDesiredSampleRate = 0;
	void *pDestBuffer = malloc(cbDestBufferSize);
	EVoiceResult result = SteamUser()->GetVoice(true, pDestBuffer, cbDestBufferSize, &nBytesWritten);
	lua_pushnumber(L, nBytesWritten);
	lua_pushnumber(L, result);
	return 2;
}

// EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate );
EXTERN int luasteam_decompressVoice(lua_State *L) {
	uint32 cbCompressed = luaL_checknumber(L, 1);
	uint32 cbDestBufferSize = luaL_checknumber(L, 2);
	uint32 nBytesWritten;
	uint32 nDesiredSampleRate = luaL_checknumber(L, 3);
	void *pCompressed = malloc(cbCompressed);
	void *pDestBuffer = malloc(cbDestBufferSize);
	EVoiceResult result = SteamUser()->DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, &nBytesWritten, nDesiredSampleRate);
	lua_pushnumber(L, nBytesWritten);
	lua_pushnumber(L, result);
	return 2;
}

// wrapper function for getting voice data directly without multiple calls
// automatically calls getAvailableVoice -> getVoice -> decompressVoice
EXTERN int luasteam_getVoiceDataDecompressed(lua_State *L) {
	uint32 pcbCompressed;
	uint32 pcbUncompressed;
	EVoiceResult result = SteamUser()->GetAvailableVoice(&pcbCompressed, &pcbUncompressed);
	if (result != k_EVoiceResultOK) {
		lua_pushnil(L);
		lua_pushnumber(L, result);
		return 2;
	}
	void *pDestBuffer = malloc(pcbCompressed);
	uint32 nBytesWritten;
	result = SteamUser()->GetVoice(true, pDestBuffer, pcbCompressed, &nBytesWritten);
	if (result != k_EVoiceResultOK) {
		lua_pushnil(L);
		lua_pushnumber(L, result);
		return 2;
	}
	uint32 cbDestBufferSize = nBytesWritten;
	void *pDecompressedBuffer = malloc(cbDestBufferSize);
	result = SteamUser()->DecompressVoice(pDestBuffer, pcbCompressed, pDecompressedBuffer, cbDestBufferSize, &nBytesWritten, 44100);
	if (result != k_EVoiceResultOK) {
		lua_pushnil(L);
		lua_pushnumber(L, result);
		return 2;
	}
	lua_pushlstring(L, (const char *)pDecompressedBuffer, nBytesWritten);
	lua_pushnumber(L, result);
	return 2;
}

EXTERN int luasteam_getVoiceData(lua_State *L) {
	uint32 pcbCompressed;
	uint32 pcbUncompressed;
	EVoiceResult result = SteamUser()->GetAvailableVoice(&pcbCompressed, &pcbUncompressed);
	if (result != k_EVoiceResultOK) {
		lua_pushnil(L);
		lua_pushnumber(L, result);
		return 2;
	}
	void *pDestBuffer = malloc(pcbCompressed);
	uint32 nBytesWritten;
	result = SteamUser()->GetVoice(true, pDestBuffer, pcbCompressed, &nBytesWritten);
	if (result != k_EVoiceResultOK) {
		lua_pushnil(L);
		lua_pushnumber(L, result);
		return 2;
	}
	lua_pushlstring(L, (const char *)pDestBuffer, nBytesWritten);
	lua_pushnumber(L, result);
	return 2;
}

namespace luasteam {

void add_user(lua_State *L) {
    lua_createtable(L, 0, 10);
    add_func(L, "getPlayerSteamLevel", luasteam_getPlayerSteamLevel);
    add_func(L, "getSteamID", luasteam_getSteamID);
    add_func(L, "loggedOn", luasteam_loggedOn);
	add_func(L, "startVoiceRecording", luasteam_startVoiceRecording);
	add_func(L, "stopVoiceRecording", luasteam_stopVoiceRecording);
	add_func(L, "getAvailableVoice", luasteam_getAvailableVoice);
	add_func(L, "getVoice", luasteam_getVoice);
	add_func(L, "decompressVoice", luasteam_decompressVoice);
	add_func(L, "getVoiceData", luasteam_getVoiceData);
	add_func(L, "getVoiceDataDecompressed", luasteam_getVoiceDataDecompressed);
    lua_setfield(L, -2, "user");
}

void init_user(lua_State *L) {}

void shutdown_user(lua_State *L) {}

} // namespace luasteam
