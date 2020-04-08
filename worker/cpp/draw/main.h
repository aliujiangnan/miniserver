#ifndef _DRAW_MAIN_H_
#define _DRAW_MAIN_H_
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "mapobj.h"

namespace draw
{
struct lambda_cb{
    static void callback(std::function<void(int id)> func, int id){
        func(id);
    }
};
struct GameMain
{
    static void cleanUp();

    static void init();

    static void onSessionReq(const int& sessionid, const int& cmd, const std::string& msg);

    static void onSessionOffline(const int& sessionid);

    static void bindHandler();

    static void bind(std::string name, std::function<void(Player* player, rapidjson::Value& data)> func);

    static int getLevel(int exp);

    static int gIndexPlayerID;

    static std::map<std::string, std::function<void(Player* player, rapidjson::Value& data)>> gMsgCallBack;
};
}
#endif
