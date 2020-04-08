#include "player.h"
#include "mapobj.h"
#include "team.h"
using namespace mini;

       
void Player::broadcastInMap(const std::string& name, const std::string& data)
{
    if (mapObj)
    {
        mapObj->foreach([&](Player* player) 
        {
            player->sendMsg(name, data);
        });
    }
}
void Player::broadcastInTeam(const std::string& name, const std::string& data)
{
    if (team){
        team->foreach([&](Player* player) {
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
