#ifndef _DRAW_PLAYER_H_
#define _DRAW_PLAYER_H_

#include <string>
#include <map>
#include <functional>
#include "../ffworker_cpp.h"

namespace draw{
class MapObj;
class Team;
class Player
{
    public:
    Player(const int& _sessionid):
    sessionid(_sessionid),
    mapObj(nullptr),
    team(nullptr),
    playerID(0),
    userID(0),
    userName(""),
    nickName(""),
    avatar(""),
    gender(0),
    score(0),
    exp(0),
    level(0),
    index(0),
    online(true),
    isPlayer(true),
    ready(false){}
    
    int getID() {return sessionid;}
    void sendMsg(const std::string& name, const std::string& data) 
    {
        rapidjson::Document dom;
        if(dom.Parse<0>(data.c_str()).HasParseError())
        {
            return;
        }
        sendMsg(name, dom);
    }
    void sendMsg(const std::string& name, rapidjson::Value& data) 
    {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("n"); writer.String(name.c_str());
        writer.String("d"); data.Accept(writer);
        writer.EndObject();
        Singleton<ff::FFWorkerCpp>::instance().sessionSendMsg(sessionid, 0, buf.GetString());
    }
    void broadcastInMap(const std::string& name, const std::string& data);
    void broadcastInTeam(const std::string& name, const std::string& data);
    void notifyOtherInTeam(const std::string& name, const std::string& data);

    void broadcastInMap(const std::string& name, rapidjson::Value& data);
    void broadcastInTeam(const std::string& name, rapidjson::Value& data);
    void notifyOtherInTeam(const std::string& name, rapidjson::Value& data);

    int sessionid;
    MapObj *mapObj;
    Team *team;
    int playerID;
    int userID;
    std::string userName;
    std::string nickName;
    std::string avatar;
    int gender;
    int score;
    int exp;
    int level;
    int index;
    bool online;
    bool isPlayer;
    bool ready;
};

struct PlayerMgr
{
    PlayerMgr(){}
    
    static PlayerMgr* getInstance();

    void addPlayer (Player* player) 
    {
        allPlayers[player->userID] = player;
        allSessions[player->getID()] = player->userID;
    }
    Player* getPlayer (const int& id)
    {
        return allPlayers[id];
    }

    Player* getPlayerBySessonId (const int& id)
    {
        int uid = allSessions[id];
        return allPlayers[uid];
    }
    bool delPlayer (const int& userID)
    {
        if (allPlayers[userID]) {
            int sessionID = allPlayers[userID]->getID();
            allPlayers.erase(userID);
            if (allSessions[sessionID] != 0) {
                allSessions.erase(sessionID);
            }
            return true;
        }
        return false;
    }
    void foreachPlayer (std::function<void (Player* player)>func) {
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second)
            {
                func(it->second);
            }
        }
    }

    std::map<int, Player*> allPlayers;
    std::map<int, int> allSessions;
    static PlayerMgr* instance;

};
}
#endif



