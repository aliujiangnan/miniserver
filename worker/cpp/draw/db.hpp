#ifndef _DRAW_DB_H_
#define _DRAW_DB_H_

#include <string>
#include <map>
#include <functional>
#include "server/db_mgr.h"

namespace draw
{
const std::string& gCfg = "sqlite://./game.db";

struct db
{
    static void query(const std::string& sql,std::function<void(ff::QueryDBResult& ret)> func = nullptr) 
    {
        ff::QueryDBResult ret;
        ff::DbMgr::instance().queryByCfg(gCfg, sql, ret);
        if (func){
            func(ret);
        }
    }

    static void createDB() 
    {
        query("create table IF NOT EXISTS user (\
            id INTEGER PRIMARY  KEY     AUTOINCREMENT,\
            username            TEXT    NOT NULL,\
            nickname            TEXT,\
            avatar              CHAR(128),\
            gender              INT\
            )");

        query("create table IF NOT EXISTS userinfo (\
            userid              INT     NOT NULL,\
            gameid              INT     NOT NULL,\
            score               INT     NOT NULL,\
            exp                 INT     NOT NULL\
            )");
    }

    static void initDB () 
    {
        createDB();
    }

    static void createUser (const std::string& username, const std::string& nickname, const int& gender, const std::string& avatar, std::function<void (int id)> func) 
    {
        query("INSERT INTO user (username,nickname,avatar,gender) VALUES (\""+ username + "\", \"" + nickname + "\", " + std::to_string(gender) + ", \"" + avatar + "\" )",
            [&] (ff::QueryDBResult& ret) {
            if (ret.errinfo != "")
            {
                func(-1);
            }
            else
            {
                query("SELECT * FROM user", [&] (ff::QueryDBResult& ret) {
                    if (ret.errinfo != "")
                    {
                        func(-1);
                    }
                    else
                    {
                        func(ret.dataResult.size());
                    }
                });
            }
        });
    }

    static void getUserInfo (const int& userid, const int& gameid, std::function<void (const std::string& data)> func) 
    {
        query("SELECT * FROM userinfo WHERE userid = " + std::to_string(userid) + " AND gameid = " + std::to_string(gameid) + "",
            [&] (ff::QueryDBResult& ret) {
            if (ret.errinfo != "")
            {
                func("");
            }
            else{
                func(ret.dataResult[0][0]);
            }
        });
    }

    static void getScore (const int& userid, const int& gameid, std::function<void (const int& score)> func) 
    {
        query("SELECT score FROM userinfo WHERE userid = " + std::to_string(userid) + " AND gameid = " + std::to_string(gameid) + "", 
            [&] (ff::QueryDBResult& ret) {
            func(0);
            // if (ret.errinfo != "")
            // {
            //     func(-1);
            // }
            // else
            // {
            //     func(std::stoi(ret.dataResult[0][0]));
            // }
        });
    }

    static void getExp (const int& userid, const int& gameid, std::function<void (const int& exp)> func) 
    {
        query("SELECT exp FROM userinfo WHERE userid = " + std::to_string(userid) + " AND gameid = " + std::to_string(gameid) + "", 
            [&] (ff::QueryDBResult& ret) {
            func(0);
            // if (ret.errinfo != "")
            // {
            //      func(-1);
            // }
            // else
            // {
            //     func(std::stoi(ret.dataResult[0][0]));
            // }
        });
    }

    static void updateScore (const int& userid, const int& gameid, const int& score) 
    {
        getScore(userid, gameid, [&] (const int& ret) 
        {
            if (ret >= 0 && ret < score) 
            {
                getUserInfo(userid, gameid, [&] (const std::string& data) {
                    if (data != "")
                    {
                        query("INSERT INTO userinfo (userid,gameid,score,exp) VALUES (" + std::to_string(userid) + ", " + std::to_string(gameid) + ", " + std::to_string(score) + ", 0)");
                    }
                    else 
                    {
                        query("UPDATE userinfo SET score = " + std::to_string(score) + " WHERE userid = " + std::to_string(userid) + " AND gameid = " + std::to_string(gameid) + "");
                    }
                });
            }
        });
    }

    static void addExp (const int& userid, const int& gameid, const int& add) {
        if (add < 0){
            return;
        }

        getUserInfo(userid, gameid, [&] (const std::string& data) {
            if (data != "")
            {
                query("INSERT INTO userinfo (userid,gameid,score,exp) VALUES (" + std::to_string(userid) + ", " + std::to_string(gameid) + ", 0, " + std::to_string(add) + ")");
            }
            else 
            {
                getExp(userid, gameid, [&] (const int& ret) {
                    if (ret >= 0) 
                    {
                        query("UPDATE userinfo SET exp = " + std::to_string(ret) + std::to_string(add) + " WHERE userid = " + std::to_string(userid) + " AND gameid = " + std::to_string(gameid) + "");
                    }
                });
            }
        });
    }

};
}
#endif

