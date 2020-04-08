#include <iostream>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "math.h"
#include "main.h"
#include "db.hpp"
#include "mapobj.h"
#include "game.h"
#include "token.h"
#include "server/ffworker.h"
using namespace draw;

int loadJson();
void bindHandler();
void showAnswer(int gameID);

void GameMain::cleanUp()
{
    printf("cpp cleanup ok");
}

void GameMain::init()
{
    db::initDB();

    loadJson();

    bindHandler();

    printf("cpp init ok");
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

rapidjson::Document jsonDom;

int loadJson() {

	const char* file_name = "worker/cpp/draw/vocabularies.json";
	std::ifstream in(file_name);
	if (!in.is_open()) {
		fprintf(stderr, "fail to read json file: %s\n", file_name);
		return -1;
	}
 
	std::string json_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    
    rapidjson::Document dom;
	if (!jsonDom.Parse<0>(json_content.c_str()).HasParseError()) 
    {
        return 0;
    }
}

void getMembers(Team* team, rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    writer.String("members"); 
    writer.StartObject();
    writer.String("players"); 
    writer.StartArray();
    team->foreachPlayer([&](const int &index, Player* playerInTeam){
        writer.StartObject();
        writer.String("id"); writer.Int(playerInTeam->userID); 
        writer.String("user"); writer.String(playerInTeam->userName.c_str()); 
        writer.String("nick"); writer.String(playerInTeam->userName.c_str()); 
        writer.String("online"); writer.Bool(playerInTeam->online); 
        writer.String("ready"); writer.Bool(playerInTeam->ready); 
        writer.String("gender"); writer.Int(playerInTeam->gender); 
        writer.String("avatar"); writer.String(playerInTeam->avatar.c_str()); 
        writer.String("score"); writer.Int(playerInTeam->score); 
        writer.String("exp"); writer.Int(playerInTeam->exp); 
        writer.String("isplayer"); writer.Int(playerInTeam->isPlayer); 
        writer.String("index"); writer.Int(playerInTeam->index); 
        writer.EndObject();
        
    });
    writer.EndArray();
    writer.String("obsers"); 
    writer.StartArray();
    team->foreach([&](Player* playerInTeam){
        if(playerInTeam->isPlayer)
        {
            return;
        }
        writer.StartObject();
        writer.String("id"); writer.Int(playerInTeam->userID); 
        writer.String("user"); writer.String(playerInTeam->userName.c_str()); 
        writer.String("nick"); writer.String(playerInTeam->userName.c_str()); 
        writer.String("online"); writer.Bool(playerInTeam->online); 
        writer.String("ready"); writer.Bool(playerInTeam->ready); 
        writer.String("gender"); writer.Int(playerInTeam->gender); 
        writer.String("avatar"); writer.String(playerInTeam->avatar.c_str()); 
        writer.String("score"); writer.Int(playerInTeam->score); 
        writer.String("exp"); writer.Int(playerInTeam->exp); 
        writer.String("isplayer"); writer.Int(playerInTeam->isPlayer); 
        writer.String("index"); writer.Int(playerInTeam->index); 
        writer.EndObject();
    });
    writer.EndArray();

    writer.EndObject();
}

void enterGame(Player* player, Team* team) {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.String("s"); writer.Int(1);
    writer.String("id"); writer.Int(player->team->teamID);
    writer.String("crter"); writer.Int(player->team->creator);
    writer.String("isplayer"); writer.Bool(player->isPlayer);
    getMembers(team, writer);
    writer.String("info");
    Game* game = GameMgr::getInstance()->getGame(team->teamID);
    if (nullptr == game)
    {
        player->isPlayer = team->getPlayerNum() <= MAX_NUM_EACH_TEAM;
        writer.String("");
    }
    else if (game->state == "selecting")
    {
        writer.StartObject();
        writer.String("state"); writer.String("selecting");
        writer.String("idx"); writer.Int(game->painterIndex);
        writer.String("t"); writer.Int(game->selectTime);
        writer.String("words");
        writer.StartArray();
        for(int i = 0; i < game->words.size();++i)
        {
            writer.StartObject();
            writer.String("tip"); writer.String(game->words[i]["tip"].c_str());
            writer.String("word"); writer.String(game->words[i]["word"].c_str());
            writer.String("idx"); writer.Int(i);
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
    }
    else if (game->state == "drawing")
    {
        writer.StartObject();
        writer.String("state"); writer.String("drawing");
        writer.String("idx"); writer.Int(game->painterIndex);
        writer.String("t"); writer.Int(game->drawTime);
        writer.String("word"); writer.String(game->words[game->wordIndex]["word"].c_str());
        writer.String("tip"); writer.String(game->words[game->wordIndex]["tip"].c_str());
        writer.String("cmds");
        writer.StartArray();
        for(int i = 0; i < game->words.size();++i)
        {
            writer.StartObject();
            writer.String("tip"); writer.String(game->words[i]["tip"].c_str());
            writer.String("word"); writer.String(game->words[i]["word"].c_str());
            writer.String("idx"); writer.Int(i);
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
    }
    else if (game->state == "showing")
    {
        writer.StartObject();
        writer.String("state"); writer.String("showing");
        writer.String("idx"); writer.Int(game->painterIndex);
        writer.String("t"); writer.Int(game->showTime);
        writer.String("word"); writer.String(game->words[game->wordIndex]["word"].c_str());
        writer.String("cmds");
        writer.StartArray();
        for(int i = 0; i < game->words.size();++i)
        {
            rapidjson::Document dom;
            if(dom.Parse<0>(game->commands[i].c_str()).HasParseError()){
                continue;
            }
            dom.Accept(writer);
        }
        writer.EndArray();
        writer.EndObject();
    }
    else if (game->state == "over")
    {
        writer.StartObject();
        writer.String("state"); writer.String("over");
        writer.String("t"); writer.Int(game->overTime);
        writer.String("scores"); writer.String("[]");
    }
    writer.EndObject();
    player->sendMsg("game.enter", buf.GetString());
    rapidjson::StringBuffer buf1;
    rapidjson::Writer<rapidjson::StringBuffer> writer1(buf1);
    writer1.StartObject();
    getMembers(team, writer1);
    writer1.String("id"); writer1.Int(player->userID);
    writer1.String("isplayer"); writer1.Bool(player->isPlayer);
    writer1.EndObject();
    player->notifyOtherInTeam("user.new", buf1.GetString());
}
void doGameOver(int gameID) 
{
    Game* game = GameMgr::getInstance()->getGame(gameID);
    if (nullptr == game)
    {
        return;
    }
    Team* team = TeamMgr::getInstance()->getTeam(gameID);
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.String("rsts");
    writer.StartArray();
    for(int i = 0; i < 6;++i)
    {
        writer.Int(game->scores[i]);
    }
    writer.EndArray();
    writer.EndObject();
    team->broadcast("game.over", buf.GetString());


    team->foreachPlayer([&] (int index, Player* player) {
        if (player->online == false) 
        {
            PlayerMgr::getInstance()->delPlayer(player->getID());
            player->team->delPlayer(player->userID);
            draw::MapObj* mapObj = player->mapObj;
            mapObj->delPlayer(player->userID);
            player->mapObj = nullptr;
        }
        else
        {
            player->ready = false;
        }
        player->score = game->scores[player->index];
        int add = player->score < 0 ? 0:player->score;
        player->exp += add;
        player->level = GameMain::getLevel(player->exp);
        db::updateScore(player->userID, GAME_ID, player->score);
        db::addExp(player->userID, GAME_ID, add);
    });
    GameMgr::getInstance()->delGame(gameID);
    game->mapObj->delGame(gameID);
    game->mapObj = nullptr;
}

std::vector<std::map<std::string,std::string>> gen(Game* game, std::vector<std::map<std::string,std::string>>& words, std::vector<int>& indics){
    rapidjson::Value& vocabularies = jsonDom["vocabularies"];

    long r1 = random();
    float r = float(r1)/float(RAND_MAX);
    int index = int(r * vocabularies.Size());
    for (int i = 0; i < indics.size(); ++i){
        if (index == indics[i])
        {
            return gen(game, words, indics);
        }
    }
    for (int i = 0; i < game->indics.size(); ++i){
        if (index == game->indics[i])
        {
            return gen(game, words, indics);
        }
    }
    std::map<std::string,std::string> value;
    value["tip"]=vocabularies[index]["type"].GetString();
    value["word"]=vocabularies[index]["word"].GetString();
    value["idx"]=std::to_string(index);
    words.push_back(value);
    if (words.size() < 4)
    {
        return gen(game, words, indics);
    }
    else
    {
        return words;
    }
}

std::vector<std::map<std::string,std::string>> genWords(int gameID)
{
    Game* game = GameMgr::getInstance()->getGame(gameID);
    std::vector<std::map<std::string,std::string>> words;
    if (nullptr == game)
    {
        return words;
    }
    
    std::vector<int> indics;
    return gen(game, words, indics);
}

void selectWord(int gameID, bool quick) {
    Game* game = GameMgr::getInstance()->getGame(gameID);
    if (nullptr == game)
    {
        return;
    }

    Team* team = TeamMgr::getInstance()->getTeam(gameID);
    if (game->painterCount < game->playNum) 
    {
        game->state = "selecting";
        timeval tv;
		gettimeofday(&tv, NULL);
        game->selectTime = tv.tv_sec;
        game->painterIndex = team->getPainterIndex(game->painterCount);
        game->painterCount += 1;
        if (game->painterIndex == -1)
        {
            return;
        }
    }
    else 
    {
        doGameOver(gameID);
        return;
    }

    Singleton<ff::FFWorkerCpp>::instance().regTimer(5000, ff::funcbind(&lambda_cb::callback, [](int id){
        Game* game = GameMgr::getInstance()->getGame(id);
        Team* team = TeamMgr::getInstance()->getTeam(id);
        game->selectTime = 0;
        if (game->wordIndex == -1)
        {
            selectWord(id, true);
        }
        else 
        {
            timeval tv;
		    gettimeofday(&tv, NULL);
            game->drawTime = tv.tv_sec;

            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            writer.StartObject();
            writer.String("idx"); writer.Int(game->painterIndex);
            writer.String("time"); writer.Int(game->drawTime);
            writer.String("word"); writer.String(game->words[game->wordIndex]["word"].c_str());
            writer.String("tip"); writer.String(game->words[game->wordIndex]["tip"].c_str());
            writer.EndObject();

            game->state = "drawing";
            game->indics.push_back(std::stoi(game->words[game->wordIndex]["idx"]));
            team->broadcast("game.draw", buf.GetString());

            Singleton<ff::FFWorkerCpp>::instance().regTimer(6000, ff::funcbind(&lambda_cb::callback, [](int id){
                showAnswer(id);
            },id));
        }
    },gameID));
    game->words = genWords(gameID);
    game->commands.clear();
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.String("idx"); writer.Int(game->painterIndex);
    writer.String("time"); writer.Int(game->selectTime);
    writer.String("words");
    writer.StartArray();
    for(int i = 0; i < game->words.size();++i)
    {
        writer.StartObject();
        writer.String("tip"); writer.String(game->words[i]["tip"].c_str());
        writer.String("word"); writer.String(game->words[i]["word"].c_str());
        writer.String("idx"); writer.Int(i);
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    team->broadcast("game.select", buf.GetString());
}

void showAnswer(int gameID) {
    Game* game = GameMgr::getInstance()->getGame(gameID);
    if (nullptr == game)
    {
        return;
    }

    Team* team = TeamMgr::getInstance()->getTeam(gameID);
    game->state = "counting";
    timeval tv;
	gettimeofday(&tv, NULL);
    game->showTime = tv.tv_sec;
    int point = 0;
    int num = 0;
    for (int i = 0; i < game->answers.size(); ++i)
    {
        if (game->answers[i]["answer"].c_str() == game->words[game->wordIndex]["word"].c_str()) 
        {
            point += 2;
            num += 2;
        }
    }
    if (num >= 5)
    {
        point = 0;
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.String("num"); writer.Int(num);
    writer.String("answer"); writer.String(game->words[game->wordIndex]["word"].c_str());
    writer.String("point"); writer.Int(point);
    writer.String("score"); writer.Int(game->scores[game->painterIndex]);
    writer.EndObject();
    team->broadcast("game.result", buf.GetString());

    Singleton<ff::FFWorkerCpp>::instance().regTimer(3000, ff::funcbind(&lambda_cb::callback, [](int id){
        Game* game = GameMgr::getInstance()->getGame(id);
        Team* team = TeamMgr::getInstance()->getTeam(id);
        game->state = "showing";
        team->broadcast("game.answer", "{}");
    },gameID));

    Singleton<ff::FFWorkerCpp>::instance().regTimer(8000, ff::funcbind(&lambda_cb::callback, [](int id){
        Game* game = GameMgr::getInstance()->getGame(id);
        game->showTime = 0;
        game->answers.clear();
        for(int i = 0; i < 6; ++i)
            game->scores[i] = 0;
        game->wordIndex = -1;
        selectWord(id, false);
    },gameID));
}

void commitAnswer(Player* player, std::string answer) {
    Game* game = GameMgr::getInstance()->getGame(player->team->teamID);
    if (nullptr == game)
    {
        return;
    }

    Team* team = player->team;
    int index = player->index;
    if (index == game->painterIndex)
    {
        return;
    }
    for (int i = 0; i < game->answers.size(); ++i)
    {
        if (answer == game->answers[i]["answer"].c_str() && index == i)
        {
            return;
        }
    }
    int scores[5] = {7, 5, 4, 3, 2};
    int score = answer != game->words[game->wordIndex]["word"].c_str() ? 0 : scores[game->answers.size()];
    std::map<std::string,std::string> value;
    value["userId"]=std::to_string(player->userID);
    value["answer"]=answer;
    value["idx"]=std::to_string(index);

    game->answers.push_back(value);
    game->scores[index] += score;
    game->scores[game->painterIndex] += 2;

    if (game->answers.size() >= 3) 
    {
        int num = 0;
        for (int i = 0; i < game->answers.size(); ++i)
        {
            if (game->answers[i]["answer"].c_str() == game->words[game->wordIndex]["word"].c_str())
            {
                num += 1;
            }
        }
        if (num == 5)
        {
            game->scores[game->painterIndex] -= 21;
        }
    }

    rapidjson::StringBuffer buf1;
    rapidjson::Writer<rapidjson::StringBuffer> writer1(buf1);
    writer1.StartObject();
    writer1.String("id"); writer1.Int(player->userID);
    writer1.String("answer"); writer1.String(answer.c_str());
    writer1.String("score"); writer1.Int(score);
    writer1.EndObject();
    player->broadcastInTeam("game.commit", buf1.GetString());
}

int GameMain::gIndexPlayerID = 1;
std::map<std::string, std::function<void(Player* player, rapidjson::Value& data)>> GameMain::gMsgCallBack;
void GameMain::bind(std::string name, std::function<void(Player* player, rapidjson::Value& data)> func){
    // printf("bind：%s", name);
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
            db::getScore(userID, GAME_ID, [&](int score){
                player->score = score;
                player->score = player->score < 0 ? 0 : player->score;
                db::getExp(userID, GAME_ID, [&](int exp){
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
                        writer.String("t"); writer.Int(tv.tv_sec);
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

    bind("invite", [](Player* player, rapidjson::Value& data)
    {        
        Team* team = player->team;
        bool create = false;
        if (nullptr == team) 
        {
            team = player->mapObj->allocTeam(player);
            team->creator = player->userID;
            create = true;
        }
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("n"); writer.String(player->nickName.c_str());
        writer.String("id"); writer.Int(player->userID);
        writer.String("team"); writer.Int(team->teamID );
        writer.EndObject();
        player->broadcastInMap("player.invite",buf.GetString());
        if (create){
            enterGame(player, team);
        }
    });

    bind("enter", [](Player* player, rapidjson::Value& data){
        Team* team = TeamMgr::getInstance()->getTeam(data["id"].GetInt());
        Game* game = GameMgr::getInstance()->getGame(team->teamID);
        if (player->team || nullptr == team || team->getMemberNum() >= MAX_NUM_EACH_TEAM + MAX_OB_EACH_TEAM)
        {
            return;
        }
        else if (game && team->getMemberNum() < MAX_NUM_EACH_TEAM)
        {
            player->team = team;
            team->addObser(player);
        }
        else 
        {
            player->team = team;
            team->addPlayer(player);
        }
        
        enterGame(player, team);
    });

    bind("ready", [](Player* player, rapidjson::Value& data){
        if (player->ready)
        {
            return;
        }
        player->ready = true;
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("id"); writer.Int(player->userID);
        writer.EndObject();
        player->broadcastInTeam("player.ready", buf.GetString());

        int n = player->team->getPlayerNum();
        printf("ready n:%d, nu:%d\n",n, player->team->getReadyNum());
        if (n > 1 && player->team->getReadyNum() == n)
        {
            player->broadcastInTeam("game.allready", "{}");
            Singleton<ff::FFWorkerCpp>::instance().regTimer(5000,ff::funcbind(&lambda_cb::callback, [](int id){
                Player* player = PlayerMgr::getInstance()->getPlayer(id);
                Game* game = new Game(player->team->teamID);
                game->mapObj = player->mapObj;
                GameMgr::getInstance()->addGame(game->gameID, game);
                player->mapObj->addGame(game->gameID, game);
                game->painterIndex = 0;
                rapidjson::StringBuffer buf;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
                writer.StartObject();
                writer.String("idx"); writer.Int(game->painterIndex);
                writer.EndObject();
                player->broadcastInTeam("game.begin", buf.GetString());
                game->state = "begin";
                game->playNum = player->team->getPlayerNum();
                selectWord(game->gameID, false);
            },player->userID));
        }
    });

    bind("standup", [](Player* player, rapidjson::Value& data){
        bool rst = player->team->standup(player);
        if (rst) 
        {
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            writer.StartObject();
            writer.String("id"); writer.Int(player->userID);
            getMembers(player->team, writer);
            writer.EndObject();
            player->broadcastInTeam("user.standup", buf.GetString());
        }
    });

    bind("sitdown", [](Player* player, rapidjson::Value& data){
        bool rst = player->team->sitdown(player);
        if (rst) 
        {
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            writer.StartObject();
            writer.String("id"); writer.Int(player->userID);
            getMembers(player->team, writer);
            writer.EndObject();
            player->broadcastInTeam("user.sitdown", buf.GetString());
        }
    });

    bind("tool", [](Player* player, rapidjson::Value& data){
        Game* game = GameMgr::getInstance()->getGame(player->team->teamID);
        if (nullptr == game)
        {
            return;
        }
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        data.Accept(writer);
        game->commands.push_back(buf.GetString());
        player->notifyOtherInTeam("cmd.tool", data);
    });

    bind("shape", [](Player* player, rapidjson::Value& data){
        Game* game = GameMgr::getInstance()->getGame(player->team->teamID);
        if (nullptr == game)
        {
            return;
        }

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        data.Accept(writer);
        game->commands.push_back(buf.GetString());
        player->notifyOtherInTeam("cmd.shape", data);
    });

    bind("select", [](Player* player, rapidjson::Value& data){
        Game* game = GameMgr::getInstance()->getGame(player->team->teamID);
        if (nullptr == game)
        {
            return;
        }
        game->wordIndex = data["idx"].GetInt();
    });

    bind("chat", [](Player* player, rapidjson::Value& data){
        player->notifyOtherInTeam("game.chat", data);
    });


    bind("commit", [](Player* player, rapidjson::Value& data){
        commitAnswer(player, data["str"].GetString());
    });


    bind("cancel", [](Player* player, rapidjson::Value& data){
        player->ready = true;
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("id"); writer.Int(player->userID);
        writer.EndObject();
        player->broadcastInTeam("player.cancel", buf.GetString());
    });

    bind("kick", [](Player* player, rapidjson::Value& data){
        Player* target = PlayerMgr::getInstance()->getPlayer(data["id"].GetInt());
        if (target) 
        {
            player->team->delPlayer(target->userID);
            target->team = nullptr;
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            writer.StartObject();
            writer.String("id"); writer.Int(target->userID);
            writer.String("online"); writer.Bool(false);
            getMembers(player->team, writer);
            writer.EndObject();
            player->team->broadcast("user.state", buf.GetString());
            target->sendMsg("game.kick", "{}");
        }
    });

    bind("exit", [](Player* player, rapidjson::Value& data){
        Team* team = player->team;
        int userID = player->userID;
        if (team->creator == userID)
        {
            return;
        }
        player->team->delPlayer(userID);
        player->team = nullptr;
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.String("id"); writer.Int(userID);
        writer.String("online"); writer.Bool(false);
        getMembers(player->team, writer);
        writer.EndObject();
        team->broadcast("user.state", buf.GetString());
        player->sendMsg("game.exit",  "{}");
    });

    bind("dissolve", [](Player* player, rapidjson::Value& data){
        if (player->userID != player->team->creator){
            return;
        }
        player->broadcastInTeam("game.exit", "{}");

        int teamID = player->team->teamID;
        player->team->foreach([&](Player* player) {
            player->team = nullptr;
        });

        TeamMgr::getInstance()->delTeam(teamID);
        player->mapObj->delTeam(teamID);
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
