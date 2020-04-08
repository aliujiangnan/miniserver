#include "player.h"
#include "mapobj.h"
#include "team.h"
using namespace draw;

void Player::broadcastInMap(const std::string& name, const std::string& data)
{
    printf("broadcastInMap1\n");
    if (mapObj)
    {
        printf("broadcastInMap2\n");
        printf("broadcastInMap2,s:%d\n",mapObj->allPlayers.size());
        mapObj->foreach([&](Player* player) 
        {
                        printf("player===== %d\n", player);
            printf("player===== %d\n", nullptr == player);
            printf("broadcastInMap3,data:%s\n",data.c_str());
            player->sendMsg(name, data);
        });
    }
}
void Player::broadcastInTeam(const std::string& name, const std::string& data)
{
    printf("broadcastInTeam1\n");
    if (team){
        printf("broadcastInTeam2\n");
        team->foreach([&](Player* player) {
            printf("broadcastInTeam3\n");
            player->sendMsg(name, data);
        });
    }
}
void Player::notifyOtherInTeam(const std::string& name, const std::string& data){
    if (team){
        team->foreach([&](Player* player) {
            if (player->getID() != sessionid) {
                player->sendMsg(name, data);
            }
        });
    }
}

void Player::broadcastInMap(const std::string& name, rapidjson::Value& data)
{
    if (mapObj)
    {
        mapObj->foreach([&](Player* player) 
        {
            player->sendMsg(name, data);
        });
    }
}
void Player::broadcastInTeam(const std::string& name, rapidjson::Value& data)
{
    if (team){
        team->foreach([&](Player* player) {
            player->sendMsg(name, data);
        });
    }
}
void Player::notifyOtherInTeam(const std::string& name, rapidjson::Value& data){
    if (team){
        team->foreach([&](Player* player) {
            if (player->getID() != sessionid) {
                player->sendMsg(name, data);
            }
        });
    }
}

PlayerMgr* PlayerMgr::instance = nullptr;
PlayerMgr* PlayerMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new PlayerMgr();
    }

    return instance;
}

