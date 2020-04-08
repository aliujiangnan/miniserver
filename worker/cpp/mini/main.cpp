
#include <iostream>
#include <fstream>
#include "math.h"
#include "main.h"
#include "db.hpp"
#include "token.h"
#include "server/ffworker.h"

using namespace mini;

void GameMain::cleanUp()
{
    printf("cpp cleanup ok\n");
}

void GameMain::init()
{
    db::initDB();

    bindHandler();

    printf("cpp init ok\n");
}


int GameMain::getLevel(int exp) 
{
    int x = exp / 40;
    if (x < 1) 
    {
        return 1;
    }
   
    return  int(log2(x)) + 2;
}

void gameBegin(Player* player, Team* team) {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
        writer.String("t"); writer.Int(1);
        writer.String("s"); writer.Int(1);
        writer.String("id"); writer.Int(team->teamID);
        writer.String("players");
        writer.StartArray();
            team->foreach([&](Player* playerInTeam){
                writer.StartObject();
                    writer.String("id"); writer.Int(playerInTeam->userID);
                    writer.String("user"); writer.String(playerInTeam->userName.c_str());
                    writer.String("nick"); writer.String(playerInTeam->userName.c_str());
                    writer.String("online"); writer.Bool(playerInTeam->online);
                    writer.String("gender"); writer.Int(playerInTeam->gender);
                    writer.String("avatar"); writer.String(playerInTeam->avatar.c_str());
                    writer.String("score"); writer.Int(playerInTeam->score);
                    writer.String("level"); writer.Int(playerInTeam->level);
                writer.EndObject();
                });
            if (team->hasRobot)
            {
                writer.StartObject();
                    writer.String("id"); writer.Int(player->userID + 1);
                    timeval tv;
                    gettimeofday(&tv, NULL);
                    writer.String("user"); writer.String(std::to_string(tv.tv_usec).c_str());
                    writer.String("nick"); writer.String(std::to_string(tv.tv_usec).c_str());
                    writer.String("online"); writer.Bool(true);
                    writer.String("gender"); writer.Int(0);
                    writer.String("avatar"); writer.String("http://sandbox-avatar.boochat.cn/2018/01/20/02/3/0042758573.gif");
                    writer.String("score"); writer.Int(0);
                    writer.String("level"); writer.Int(1);
                writer.EndObject();
            }
            writer.EndArray();
        writer.String("robot"); writer.Int(team->hasRobot ? 1 : 0);
    writer.EndObject();
    player->broadcastInTeam("game.begin", buf.GetString());
}

int GameMain::gIndexPlayerID = 1;

std::map<std::string, std::function<void(Player* player, rapidjson::Value& data)>> GameMain::gMsgCallBack;
void GameMain::bind(std::string name, std::function<void(Player* player, rapidjson::Value& data)> func){
    // printf("bind：%s\n", name);
    gMsgCallBack[name] = func;
}


void GameMain::bindHandler(){
    bind("login", [](Player* player, rapidjson::Value& data)
    {
        int id = -1;
        if (data.HasMember("d") && data["d"].IsInt()) 
        {
            id = data["d"].GetInt();
        }
        int userID = id;
        player->userName = data["user"].GetString();
        player->nickName = data["nick"].GetString();
        player->avatar = data["avatar"].GetString();
        player->gender = data["gender"].GetInt();

        std::function<void(const int& userID)> callback = [&](const int& userID)
        {
            if (userID < 0) 
            {
                return;
            }
            player->userID = userID;
            gIndexPlayerID += 1;
            PlayerMgr::getInstance()->addPlayer(player);
            player->mapObj = MapObjMgr::getInstance()->allocMap();
            player->mapObj->addPlayer(player);
            player->playerID = player->mapObj->allocPlayerID();
            int gameType = data["game"].GetInt();
            db::getScore(userID, gameType, [&](int score){
                player->score = score;
                player->score = player->score < 0 ? 0 : player->score;
                db::getExp(userID, gameType, [&](int exp){
                    player->exp = exp;
                    player->exp = player->exp < 0 ? 0 : player->exp;
                    player->level = getLevel(player->exp);
                    std::string token = TokenMgr::getInstance()->newToken(userID, 7*24*3600*1000);
                    printf("send init data\n");

                    rapidjson::StringBuffer buf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
                    writer.StartObject();
                        writer.String("token"); writer.String(token.c_str());
                        timeval tv;
                        gettimeofday(&tv, NULL);
                        writer.String("t"); writer.Int(tv.tv_usec);
                        writer.String("id"); writer.Int(userID);
                        writer.String("score"); writer.Int(player->score);
                        writer.String("exp"); writer.Int(player->exp);
                        writer.String("lv"); writer.Int(player->level);
                    writer.EndObject();

                    player->sendMsg("game.init", buf.GetString());
                });
            });
        };

        if (userID == -1)
        {
            db::createUser(player->userName,player->nickName,player->gender,player->avatar, [&](int userid){callback(userid);});
        }
        if (userID == -1)
        {
            return;
        }
        callback(gIndexPlayerID);
    });

    bind("match", [](Player* player, rapidjson::Value& data)
    {
        printf("matching\n");
        Team* team = player->mapObj->MatchTeam(player, data["type"].GetInt());
        printf("teamid, %d, num %d\n", team->teamID, team->getMemberNum());

        if (team->getMemberNum() == MAX_NUM_EACH_TEAM)
        {
            gameBegin(player, team);
        }
        
        Singleton<ff::FFWorkerCpp>::instance().regTimer(15000, ff::funcbind(&lambda_cb::callback, [](int id){
            Player* player = PlayerMgr::getInstance()->getPlayer(id);
            Team* team = player->team;
            if (team->getMemberNum() == MAX_NUM_EACH_TEAM || team->hasRobot) 
            {
                return;
            }
            team->hasRobot = true;
            gameBegin(player, team);
        }, player->userID));
    });

    
    bind("msg", [](Player* player, rapidjson::Value& data){
        player->notifyOtherInTeam("player.msg", data);
    });

    bind("bcast", [](Player* player, rapidjson::Value& data){
        player->broadcastInTeam("player.bcast", data);
    });

    bind("emoji", [](Player* player, rapidjson::Value& data){
        player->notifyOtherInTeam("player.emoji", data);
    });

    bind("again", [](Player* player, rapidjson::Value& data){
        player->notifyOtherInTeam("player.again", data);
    });

    bind("accept", [](Player* player, rapidjson::Value& data){
        gameBegin(player, player->team);
    });

    bind("dissolve", [](Player* player, rapidjson::Value& data){
        player->broadcastInTeam("game.dissolve", "{}");

        TeamMgr::getInstance()->delTeam(player->team->teamID);
        player->mapObj->delTeam(player->team->teamID);
        player->team = nullptr;
    });

    bind("gameover", [](Player* player, rapidjson::Value& data){
        int rst = data["rst"].GetInt();
        int deleteIds[2] = {-1,-1};
        int index = 0;
        Team* team = player->team;
                        MapObj* mapObj =player->mapObj;

        player->team->foreach([&](Player* playerInTeam){
            int result = 0;
            if (playerInTeam->userID == player->userID)
            {
                result = rst;
            }
            else if (rst == 2)
            {
                result = rst;
            }
            else
            {
                result = rst == 0 ? 1 : 0;
            }
            int expAdd =result == 0 ? 20 : 40;
            int scoreAdd =result == 0? -10 : 20;
            int beforeNext =int(40 * pow(2, playerInTeam->level)) - int(40 * pow(2, playerInTeam->level - 1));
            int beforeLast =playerInTeam->level == 1 ? 0 : int(40 * pow(2, playerInTeam->level - 1)) - int(40 * pow(2, player->level - 2));
            int beforeExp =playerInTeam->exp - beforeLast;
            int curExp =playerInTeam->exp + expAdd;
            int curLv =getLevel(curExp);
            int curScore =playerInTeam->score + scoreAdd;
            curScore = curScore < 0? 0 : curScore;
            int nextExp =int(40 * pow(2, curLv)) - int(40 * pow(2, curLv - 1));
            int lastExp =curLv == 1 ? 0 : int(40 * pow(2, curLv - 1)) - int(40 * pow(2, curLv - 2));
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            writer.StartObject();
                writer.String("rst"); writer.Int(result);
                writer.String("data");
                writer.StartObject();
                    writer.String("curlv"); writer.Int(curLv);
                    writer.String("curscore"); writer.Int(curScore);
                    writer.String("befoexp"); writer.Int(beforeExp);
                    writer.String("befonext"); writer.Int(beforeNext);
                    writer.String("curexp"); writer.Int(curExp - lastExp);
                    writer.String("nextexp"); writer.Int(nextExp);
                    writer.String("scoreadd"); writer.String(((scoreAdd >= 0 ? "+" : "") + std::to_string(scoreAdd)).c_str());
                    writer.String("expadd"); writer.String(("+" + std::to_string(expAdd)).c_str());
                writer.EndObject();
            writer.EndObject();
            player->score = curScore;
            player->exp = curExp;
            player->level = curLv;
            db::updateScore(player->userID, player->team->selfType, player->score);
            db::addExp(player->userID, player->team->selfType, expAdd);
            playerInTeam->sendMsg("game.over", buf.GetString());
            index++;
            if (playerInTeam->online == false)
            {
                PlayerMgr::getInstance()->delPlayer(playerInTeam->userID);
                deleteIds[index] = playerInTeam->userID;
                mapObj->delPlayer(playerInTeam->userID);
                playerInTeam->mapObj = nullptr;
            }
        });
        for(int i = 0; i < 2;++i)
        {
            if(deleteIds[i] >= 0)
            {
                team->delPlayer(deleteIds[i]);
            }
        }
        if (team->getMemberNum() < MAX_NUM_EACH_TEAM)
        {
            TeamMgr::getInstance()->delTeam(team->teamID);
            mapObj->delTeam(team->teamID);
            player->team = nullptr;
        }
    });

    bind("ping", [](Player* player, rapidjson::Value& data){
        player->sendMsg("game.pong", "{}");
    });

}

void GameMain::onSessionReq(const int& sessionid, const int& cmd, const std::string& msg){
    printf("onSessionReq: id,%d,msg,%s\n",sessionid,msg.c_str());
    
    rapidjson::Document dom;
    if(dom.Parse<0>(msg.c_str()).HasParseError())
    {
        return;
    }

    std::string name = dom["n"].GetString();
    rapidjson::Value data;
    if (dom.HasMember("d") && dom["d"].IsObject()) 
    {
		data = dom["d"];
	}

    std::function<void (Player* player, rapidjson::Value& data)> func = gMsgCallBack[name];
    // printf(name.c_str());
    if (func)
    {
        auto player = PlayerMgr::getInstance()->getPlayerBySessonId(sessionid);
        if (nullptr == player)
        {
            if (name == "login")
            {
                // if (!TokenMgr::getInstance()->isValid(data["sign"].GetString()) && data["sign"].GetString() != "notoken")
                // {
                //     printf("sign error\n");
                //     return;
                // }
                player = new Player(sessionid);
            }
            else
            {
                return;
            }
        }
        func(player, data);
    }
}

void GameMain::onSessionOffline(const int& sessionid)
{
    printf("onSessionOffline: id,%d\n", sessionid);
    Player* player = PlayerMgr::getInstance()->getPlayerBySessonId(sessionid);
    if (nullptr == player)
    {
        return;
    }

    if (player->team) 
    {
        MapObj* mapObj = player->mapObj;
        PlayerMgr::getInstance()->delPlayer(player->userID);
        mapObj->delPlayer(player->userID);
        player->mapObj = nullptr;
    }
    else
    {
        player->online = false;

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("id"); writer.Int(player->userID);
        writer.EndObject();

        player->notifyOtherInTeam("user.offline", buf.GetString());
    }
}
