#ifndef _DRAW_TEAM_H_
#define _DRAW_TEAM_H_

#include <string>
#include <map>
#include <functional>
#include "common.h"
#include "player.h"

namespace draw
{
struct Team
{
    Team(const int& _teamID):
    teamID(_teamID),
    selfType(0),
    creator(0){}
    
    void addPlayer(Player* player)
    {
        player->index = getMemberNum();
        allPlayers[player->userID] = player;
    }
    Player* getPlayer(const int& userID) 
    {
        return allPlayers[userID];
    }
    Player* getPlayerByIndex(const int& index)
    {
        int i = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if (i == it->second->index)
            {
                return it->second;
            }
            i += 1;
        }
        return nullptr;
    }
    bool delPlayer(const int& userID) 
    {
        if (allPlayers[userID]) 
        {
            allPlayers.erase(userID);
            return true;
        }
        return false;
    }
    void addObser(Player* player)
    {
        player->index = getMemberNum();
        allObsers[player->userID] = player;
    }
    Player* getObser(const int& userID)
    {
        return allObsers[userID];
    }
    bool delObser(const int& userID) 
    {
        if (allObsers[userID])
        {
            allObsers.erase(userID);
            return true;
        }
        return false;
    }
    int getMemberNum()
    {
        return getPlayerNum() + getObserNum();
    }
    int getPlayerNum()
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
    int getObserNum()
    {
        int i = 0;
        std::map<int, Player*>::iterator it = allObsers.begin();
        for (; it != allObsers.end(); ++it)
        {
            if(it->second)
            {
                ++i;
            }
        }
        return i;
    }

    int getReadyNum()
    {
        int i = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if (it->second && it->second->ready)
            {
                i += 1;
            }
        }
        return i;
    }
    bool standup(Player* player) 
    {
        if (getObserNum() < MAX_OB_EACH_TEAM)
        {
            delPlayer(player->userID);
            addObser(player);
            player->isPlayer = false;
            player->ready = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    bool sitdown(Player* player) 
    {
        if (getPlayerNum() < MAX_NUM_EACH_TEAM)
        {
            delObser(player->userID);
            addPlayer(player);
            player->isPlayer = true;
            player->ready = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    void foreach(std::function<void(Player *player)>func)
    {
        std::map<int, Player*>::iterator pit = allPlayers.begin();
        for (; pit != allPlayers.end(); ++pit)
        {
            if(pit->second)
            {
                func(pit->second);
            }
        }
        std::map<int, Player*>::iterator oit = allObsers.begin();
        for (; oit != allObsers.end(); ++oit)
        {
            if(oit->second)
            {
                func(oit->second);
            }
        }
    }
    void foreachPlayer(std::function<void(const int& index, Player* player)> func)
    {
        int i = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second)
            {
                func(i, it->second);
                i += 1;
            }
        }
    }
    void broadcast(const std::string& name, const std::string& data)
    {
        foreach([&](Player* player){
            player->sendMsg(name, data);
        });
    }
    int getOlPlayerNum() 
    {
        int n = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second->online){
                n += 1;
            }
        }
        return n;
    }
    int getPainterIndex(const int& count) {
        int index = -1;
        int n = 0;
        int i = 0;
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second->online){
                if (n == count){
                    index = i;
                    break;
                }
                n += 1;
            }
            i += 1;
        }
        return index;
    }

    int teamID;
    int selfType;
    int creator;
    std::map<int, Player*> allPlayers;
    std::map<int, Player*> allObsers;

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
