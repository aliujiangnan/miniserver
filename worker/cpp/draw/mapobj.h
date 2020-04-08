#ifndef _DRAW_MAPOBJ_H_
#define _DRAW_MAPOBJ_H_

#include <map>
#include <functional>
#include "common.h"
#include "team.h"
#include "game.h"

namespace draw
{
struct MapObj
{
    MapObj(const int& _mapid):mapid(_mapid){}
    ~MapObj(){
        std::map<int, Player*>::iterator pit = allPlayers.begin();
        for (; pit != allPlayers.end(); ++pit)
        {
            if(nullptr != pit->second)
            {
                Player* player = pit->second;
                allPlayers.erase(pit->first);
                delete player;
            }
        }

        std::map<int, Team*>::iterator tit = allTeams.begin();
        for (; tit != allTeams.end(); ++tit)
        {
            if(nullptr != tit->second)
            {
                Team* team = tit->second;
                allTeams.erase(tit->first);
                delete team;
            }
        }

        std::map<int, Game*>::iterator git = allGames.begin();
        for (; git != allGames.end(); ++git)
        {
            if(nullptr != git->second)
            {
                Game* game = git->second;
                allGames.erase(git->first);
                delete game;
            }
        }
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
        Player* player = allPlayers[userID];
        if (allPlayers[userID]) 
        {
            allPlayers.erase(userID);
            TeamMgr::getInstance()->foreachTeam([&](Team* team) 
            {
                team->delPlayer(userID);
            });

            delete player;
            return true;
        }
        return false;
    }
    bool foreach(std::function<void (Player* player)> func) 
    {
        std::map<int, Player*>::iterator it = allPlayers.begin();
        for (; it != allPlayers.end(); ++it)
        {
            if(it->second)
            {
                func(it->second);
            }
        }
        return true;
    }
    int allocPlayerID()
    {
        for (int i = 1;i <= 100; ++i) 
        {
            if (allPlayers[i])
            {
                return i;
            }
        }
        return 1;
    }
    int getTeamNum() 
    {
        int i = 0;
        std::map<int, Team*>::iterator it = allTeams.begin();
        for (; it != allTeams.end(); ++it)
        {
            if(it->second)
            {
                ++i;
            }
        }
        return i;
    }
    void addTeam(Team* team) 
    {
        allTeams[team->teamID] = team;
    }
    bool delTeam(const int& teamID)
    {
        Team* team = allTeams[teamID];
        if (allTeams[teamID]) 
        {
            allTeams.erase(teamID);
            delete team;
            return true;
        }
        return false;
    }
    Team* allocTeam(Player* player) {
        Team* retTeam = new Team(allocTeamID());
        player->team = retTeam;
        retTeam->addPlayer(player);
        addTeam(retTeam);
        TeamMgr::getInstance()->addTeam(retTeam);
        return retTeam;
    }
    int allocTeamID() 
    {
        for (int i = 1;i <= 100; ++i)
        {
            if (allTeams[i])
            {
                return i;
            }
            return 1;
        }

        return 1;
    }
    void addGame(const int& gameID, Game* game)
    {
        allGames[gameID] = game;
    }

    bool delGame(const int& gameID) 
    {
        Game* game = allGames[gameID];
        if (allGames[gameID])
        {
            allGames.erase(gameID);
            delete game;
            return true;
        }
        return false;
    }

    int mapid;
    std::map<int, Player*> allPlayers;
    std::map<int, Team*> allTeams;
    std::map<int, Game*> allGames;
};

struct MapObjMgr
{
    MapObjMgr(){}
    ~MapObjMgr(){
        std::map<int, MapObj*>::iterator it = allMaps.begin();
        for (; it != allMaps.end(); ++it)
        {
            if(nullptr != it->second)
            {
                MapObj* mapObj = it->second;
                allMaps.erase(it->first);
                delete mapObj;
            }
        }
    }

    static MapObjMgr* getInstance();

    MapObj* allocMap () 
    {
        MapObj* retMap = nullptr;
        std::map<int, MapObj*>::iterator it = allMaps.begin();
        for (; it != allMaps.end(); ++it)
        {
            if(it->first >= 0 && it->second->getPlayerNum() < MAX_NUM_EACH_MAP)
            {
                retMap = it->second;
                break;
            }
        }
        
        if (nullptr == retMap)
        {
            retMap = new MapObj(allocID());
            allMaps[retMap->mapid] = retMap;
        }
        return retMap;
    }

    std::map<int, MapObj*> allMaps;
    static MapObjMgr* instance;

    int gID = 0;
    int allocID() 
    {
        gID += 1;
        return gID;
    }
};
}
#endif

