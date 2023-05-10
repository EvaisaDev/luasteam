#include "utils.hpp"
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::cout; using std::endl;
using std::cin; using std::string;
// ==========================
// ======= SteamUtils =======
// ==========================

// uint32 GetAppID();
EXTERN int luasteam_getAppID(lua_State *L) {
    lua_pushnumber(L, SteamUtils()->GetAppID());
    return 1;
}

string uint64_ToBase(uint64 x, char *chars)
{
    string result;
    int chars_count = strlen(chars);
    while (x) {
        result += chars[x % chars_count];
        x /= chars_count;
    }

    return result.c_str();
}

uint64 uint64_FromBase(string x, char *chars)
{
    uint64 result{};
    int chars_count = strlen(chars);
    while (!x.empty()) {
        auto iter = std::find(chars, chars + chars_count, x.back());
        int index = iter - chars;
        x.pop_back();

        result *= chars_count;
        result += index;
    }
    return result;
}

// Implement run length encoding but only encode when 2 or more characters are in a row
string encode(string input){
    string output = "";
    int count = 0;
    for(int i = 0; i < input.length(); i++){
        if(input[i] == input[i+1]){
            count++;
        }
        else{
            if(count > 1){
                output += std::to_string(count) + input[i];
                count = 0;
            }
            else{
                output += input[i];
            }
        }
    }
    return output;
}

string decode(string input){
    string output = "";
    int count = 0;
    for(int i = 0; i < input.length(); i++){
        if(input[i] >= '0' && input[i] <= '9'){
            count = count * 10 + (input[i] - '0');
        }
        else{
            for(int j = 0; j < count; j++){
                output += input[i];
            }
            count = 0;
        }
    }
    return output;
}

typedef unsigned char uchar;
static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";//=
static std::string base64_encode(const std::string &in) {
    std::string out;

    int val=0, valb=-6;
    for (uchar c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back(b[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back(b[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}


static std::string base64_decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T[b[i]] = i;

    int val=0, valb=-8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}

void setClipboard (const char* str )
{ 
    const size_t len = strlen(str) + 1;
    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), str, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

char* getClipboard(){
    if(OpenClipboard(NULL)){
        HANDLE hData = GetClipboardData(CF_TEXT);
        if(hData == NULL) return NULL;
        char* buffer = (char*)GlobalLock(hData);
        GlobalUnlock(hData);
        CloseClipboard();
        return buffer;
    }
    return NULL;
}

EXTERN int luasteam_compressSteamID(lua_State *L) {
    const uint64 id = luasteam::checkuint64(L, 1);
    
    lua_pushstring(L, uint64_ToBase(id, "ABCDEFGHIJKLMNOPQRSTUVWXYZ").c_str());


    return 1;
}

EXTERN int luasteam_decompressSteamID(lua_State *L){
    const char *id = luaL_checkstring(L, 1);
    uint64 result = uint64_FromBase(id, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");


    luasteam::pushuint64(L, result);
    return 1;
}

EXTERN int luasteam_setClipboard(lua_State *L){
    const char *str = luaL_checkstring(L, 1);
    setClipboard(str);
    return 0;
}

EXTERN int luasteam_getClipboard(lua_State *L){
    char *str = getClipboard();
    lua_pushstring(L, str);
    return 1;
}


EXTERN int luasteam_getUnixTimeStamp(lua_State *L){

    // Get current system time in UTC
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert UTC system time to Unix timestamp
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    ULONGLONG ft64 = ull.QuadPart;
    time_t unix_time = (ft64 - 116444736000000000ULL) / 10000000;

    // Get current system time in milliseconds
    ULONGLONG system_time_ms = ft64 / 10000;

    // Subtract Unix timestamp from system time in milliseconds to get milliseconds since Unix epoch
    ULONGLONG unix_time_ms = system_time_ms - (11644473600000ULL);

    // convert to uint64_t
    uint64_t unix_time_ms_64 = unix_time_ms;

    luasteam::pushuint64(L, unix_time_ms_64);

    return 1;
}

EXTERN int luasteam_uintToString(lua_State *L){
    uint64_t number = luasteam::checkuint64(L, 1);

    std::string str = std::to_string(number);

    lua_pushstring(L, str.c_str());

    return 1;
}

EXTERN int luasteam_stringToUint(lua_State *L){
    const char *str = luaL_checkstring(L, 1);

    uint64_t number = std::stoull(str);

    luasteam::pushuint64(L, number);

    return 1;
}

EXTERN int luasteam_getUnixTimeElapsed(lua_State *L){
    uint64_t time1 = luasteam::checkuint64(L, 1);
    uint64_t time2 = luasteam::checkuint64(L, 2);

    // return time elapsed as a 32 bit integer
    lua_pushinteger(L, time2 - time1);

    return 1;
}

const char *keyboard_modes[] = {"SingleLine", "MultipleLines", "Email", "Numeric", nullptr};

//bool ShowFloatingGamepadTextInput( EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight );
EXTERN int luasteam_showFloatingGamepadTextInput(lua_State *L){
    //static_cast<EFloatingGamepadTextInputMode>(luaL_checkoption(L, 1, nullptr, keyboard_modes))
    SteamUtils()->ShowFloatingGamepadTextInput(static_cast<EFloatingGamepadTextInputMode>(luaL_checkoption(L, 1, nullptr, keyboard_modes)), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5));

    return 0;
}

EXTERN int luasteam_loggedOn(lua_State *L){
    lua_pushboolean(L, SteamUser()->BLoggedOn());
    return 1;
}

namespace luasteam {

void add_utils(lua_State *L) {
    lua_createtable(L, 0, 11);
    add_func(L, "getAppID", luasteam_getAppID);
    add_func(L, "compressSteamID", luasteam_compressSteamID);
    add_func(L, "decompressSteamID", luasteam_decompressSteamID);
    add_func(L, "setClipboard", luasteam_setClipboard);
    add_func(L, "getClipboard", luasteam_getClipboard);
    add_func(L, "getUnixTimeStamp", luasteam_getUnixTimeStamp);
    add_func(L, "getUnixTimeElapsed", luasteam_getUnixTimeElapsed);
    add_func(L, "uintToString", luasteam_uintToString);
    add_func(L, "stringToUint", luasteam_stringToUint);
    add_func(L, "showFloatingGamepadTextInput", luasteam_showFloatingGamepadTextInput);
    add_func(L, "loggedOn", luasteam_loggedOn);
    lua_setfield(L, -2, "utils");
}

void init_utils(lua_State *L) {}

void shutdown_utils(lua_State *L) {}

} // namespace luasteam
