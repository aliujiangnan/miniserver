#ifndef _MINI_TEAM_H_
#define _MINI_TEAM_H_

#include <string>
#include <map>
#include <functional>
#include "common.h"
#include "player.h"

namespace mini
{
struct Team
{
    Team(const int& _teamID):
    teamID(_teamID),
    selfType(0),
    hasRobot(false){}
    
    void addPlayer(Player* player)
    {
        allPlayers[player->userID] = player;
    }
    Player* getPlayer(const int& userID) 
    {
        return allPlayers[userID];
    }
    bool delPlayer(const int& userID) 
    {
        auto p = allPlayers[userID];
        if (allPlayers[userID]) 
        {
            allPlayers.erase(userID);
            return true;
        }
        return false;
    }
    int getMemberNum()
    {
        int i = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second)
            {
                ++i;
            }
        }
        return i;
    }
    void foreach(std::function<void(Player *player)>func)
    {
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second)
            {
                func(it->second);
            }
        }
    }
    void broadcast(const std::string& name, const std::string& data)
    {
        foreach([&](Player* player){
            player->sendMsg(name, data);
        });
    }
    void broadcast(const std::string& name, rapidjson::Value& data)
    {
        foreach([&](Player* player){
            player->sendMsg(name, data);
        });
    }
    int teamID;
    int selfType;
    bool hasRobot;
    std::map<int, Player*> allPlayers;

};

struct TeamMgr
{
    TeamMgr(){}

    static TeamMgr* getInstance();

    void addTeam(Team* team)
    {
        allTeams[team->teamID] = team;
    }

    Team* getTeam(const int& id) 
    {
        return allTeams[id];
    }

    bool delTeam(const int& teamID) 
    {
        if (allTeams[teamID]) 
        {
            allTeams.erase(teamID);
            return true;
        }
        return false;
    }

    void foreachTeam(std::function<void (Team* team)> func)
    {
        std::map<int, Team*>::iterator it = allTeams.begin();
        for (; it != allTeams.end(); ++it)
        {
            if(it->second)
            {
                func(it->second);
            }
        }
    }

    std::map<int, Team*> allTeams;
    static TeamMgr* instance;
};
}
#endif
