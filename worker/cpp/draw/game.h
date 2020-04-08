#ifndef _DRAW_GAME_H_
#define _DRAW_GAME_H_

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "rapidjson/document.h"

class MapObj;

namespace draw
{
struct Game
{
    Game(const int& _gameID):
    gameID(_gameID),
    mapObj(nullptr),
    state(""),
    painterIndex(-1),
    wordIndex(-1),
    painterCount(0),
    selectTime(0),
    drawTime(0),
    showTime(0),
    endTime(0),
    overTime(0),
    playNum(0){}

    int gameID;
    MapObj *mapObj;
    std::string state;
    int painterIndex;
    int wordIndex;
    int painterCount;
    int selectTime;
    int drawTime;
    int showTime;
    int endTime;
    int overTime;
    int playNum;
    int scores[6] = {};
    std::vector<int> indics;
    std::vector<std::string> commands;
    std::vector<std::map<std::string,std::string>> words;
    std::vector<std::map<std::string,std::string>> answers;
};

struct GameMgr
{
    GameMgr(){}
        
    static GameMgr* getInstance();
    
    void addGame(const int& gameID, Game* game) 
    {
        allGames[gameID] = game; 
    }
    Game* getGame(const int& gameID)
    {
        return allGames[gameID];
    }
    bool delGame(const int& gameID)
    {
        if (allGames[gameID])
        {
            allGames.erase(gameID);  
            return true;
        }
        return false;
    }
    void foreachGame(std::function<void (Game* game)> func) {
        std::map<int, Game*>::iterator it = allGames.begin();
        for (; it != allGames.end(); ++it)
        {
            if(it->second)
            {
                func(it->second);
            }
        }
    }

    std::map<int, Game*> allGames;
    static GameMgr* instance;

};
}
#endif

